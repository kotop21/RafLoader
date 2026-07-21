#include "RenderHook.h"
#include "ConsoleState.h"
#include "ImGuiMenu.h"

#include <MinHook.h>
#include <d3d9.h>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <vector>
#include <windows.h>

extern void InitLuaEngine();

typedef void (*TickCallback_t)();

std::vector<TickCallback_t> g_TickCallbacks;

extern "C" __declspec(dllexport) void __cdecl
Core_RegisterTickCallback(TickCallback_t cb) {
  if (cb)
    g_TickCallbacks.push_back(cb);
}

typedef HRESULT(APIENTRY *Present_t)(IDirect3DDevice9 *, const RECT *,
                                     const RECT *, HWND, const RGNDATA *);

typedef HRESULT(APIENTRY *Reset_t)(IDirect3DDevice9 *, D3DPRESENT_PARAMETERS *);

typedef void(__fastcall *GameLoop_t)(void *, void *, int32_t *);

Present_t oPresent = nullptr;
Reset_t oReset = nullptr;
GameLoop_t oGameLoop = nullptr;

bool g_ImGuiInitialized = false;
WNDPROC oWndProc = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam,
                          LPARAM lParam) {
  if (uMsg == WM_KEYDOWN && wParam == VK_F1) {
    GIsConsoleOpen = !GIsConsoleOpen;
    return TRUE;
  }

  if (GIsConsoleOpen &&
      ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
    return TRUE;

  if (GIsConsoleOpen && (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN ||
                         uMsg == WM_MOUSEWHEEL || uMsg == WM_MOUSEMOVE))
    return TRUE;

  return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT APIENTRY hkReset(IDirect3DDevice9 *pDevice,
                         D3DPRESENT_PARAMETERS *pPresentationParameters) {

  if (g_ImGuiInitialized)
    ImGui_ImplDX9_InvalidateDeviceObjects();

  HRESULT hr = oReset(pDevice, pPresentationParameters);

  if (g_ImGuiInitialized && SUCCEEDED(hr))
    ImGui_ImplDX9_CreateDeviceObjects();

  return hr;
}

void __fastcall hkGameLoop(void *ecx, void *edx, int32_t *arg1) {

  std::vector<TickCallback_t> callbacks = g_TickCallbacks;

  for (auto cb : callbacks) {
    if (cb)
      cb();
  }

  oGameLoop(ecx, edx, arg1);
}

HRESULT APIENTRY hkPresent(IDirect3DDevice9 *pDevice, const RECT *pSourceRect,
                           const RECT *pDestRect, HWND hDestWindowOverride,
                           const RGNDATA *pDirtyRegion) {

  if (!g_ImGuiInitialized) {

    D3DDEVICE_CREATION_PARAMETERS params{};

    if (FAILED(pDevice->GetCreationParameters(&params)))
      return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride,
                      pDirtyRegion);

    HWND hWindow = params.hFocusWindow;

    if (!hWindow)
      hWindow = GetForegroundWindow();

    if (!hWindow)
      return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride,
                      pDirtyRegion);

    oWndProc =
        (WNDPROC)SetWindowLongPtr(hWindow, GWL_WNDPROC, (LONG_PTR)WndProc);

    if (!oWndProc)
      return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride,
                      pDirtyRegion);

    ImGui::CreateContext();

    if (!ImGui_ImplWin32_Init(hWindow)) {

      ImGui::DestroyContext();
      return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride,
                      pDirtyRegion);
    }

    if (!ImGui_ImplDX9_Init(pDevice)) {
      ImGui_ImplWin32_Shutdown();
      ImGui::DestroyContext();
      return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride,
                      pDirtyRegion);
    }

    g_ImGuiInitialized = true;

    InitLuaEngine();
  }

  IDirect3DStateBlock9 *stateBlock = nullptr;

  if (SUCCEEDED(pDevice->CreateStateBlock(D3DSBT_ALL, &stateBlock))) {
    stateBlock->Capture();
  }

  ImGui_ImplDX9_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  DrawImGuiMenu();

  ImGui::EndFrame();
  ImGui::Render();

  if (SUCCEEDED(pDevice->BeginScene())) {
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    pDevice->EndScene();
  }

  if (stateBlock) {
    stateBlock->Apply();
    stateBlock->Release();
  }
  return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride,
                  pDirtyRegion);
}

void InitRenderHook() {
  HWND dummyWindow = CreateWindowA(
      "BUTTON", "Dummy", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT,
      CW_USEDEFAULT, 300, 300, nullptr, nullptr, nullptr, nullptr);

  if (!dummyWindow)
    return;

  IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);

  if (!d3d) {
    DestroyWindow(dummyWindow);
    return;
  }

  D3DPRESENT_PARAMETERS d3dpp{};
  d3dpp.Windowed = TRUE;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.hDeviceWindow = dummyWindow;

  IDirect3DDevice9 *dummyDevice = nullptr;

  HRESULT hr = d3d->CreateDevice(
      D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, dummyWindow,
      D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dummyDevice);

  if (FAILED(hr) || !dummyDevice) {
    d3d->Release();
    DestroyWindow(dummyWindow);
    return;
  }

  void **vTable = *reinterpret_cast<void ***>(dummyDevice);

  void *presentAddress = vTable[17];
  void *resetAddress = vTable[16];
  void *updateSceneAddress = (void *)0x0075D8C0;

  if (MH_CreateHook(presentAddress, (LPVOID)hkPresent,
                    reinterpret_cast<LPVOID *>(&oPresent)) != MH_OK) {

    dummyDevice->Release();
    d3d->Release();
    DestroyWindow(dummyWindow);
    return;
  }

  if (MH_CreateHook(resetAddress, (LPVOID)hkReset,
                    reinterpret_cast<LPVOID *>(&oReset)) != MH_OK) {

    MH_RemoveHook(presentAddress);

    dummyDevice->Release();
    d3d->Release();
    DestroyWindow(dummyWindow);
    return;
  }

  if (MH_CreateHook(updateSceneAddress, (LPVOID)hkGameLoop,
                    reinterpret_cast<LPVOID *>(&oGameLoop)) != MH_OK) {

    MH_RemoveHook(presentAddress);
    MH_RemoveHook(resetAddress);

    dummyDevice->Release();
    d3d->Release();
    DestroyWindow(dummyWindow);
    return;
  }

  if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {

    MH_RemoveHook(presentAddress);
    MH_RemoveHook(resetAddress);
    MH_RemoveHook(updateSceneAddress);

    dummyDevice->Release();
    d3d->Release();
    DestroyWindow(dummyWindow);
    return;
  }

  dummyDevice->Release();
  d3d->Release();

  DestroyWindow(dummyWindow);
}
