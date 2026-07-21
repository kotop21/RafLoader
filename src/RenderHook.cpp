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
  g_TickCallbacks.push_back(cb);
}

typedef void (*InputCallback_t)(uint32_t uMsg, uintptr_t wParam);
std::vector<InputCallback_t> g_InputCallbacks;

extern "C" __declspec(dllexport) void __cdecl
Core_RegisterInputCallback(InputCallback_t cb) {
  g_InputCallbacks.push_back(cb);
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

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg,
                                              WPARAM wParam, LPARAM lParam);

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam,
                          LPARAM lParam) {
  if (uMsg == WM_KEYDOWN && wParam == 0x70) {
    GIsConsoleOpen = !GIsConsoleOpen;
    return true;
  }
  if (GIsConsoleOpen &&
      ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
    return true;
  if (GIsConsoleOpen && (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN ||
                         uMsg == WM_MOUSEWHEEL))
    return true;

  if (!GIsConsoleOpen && (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP)) {
    for (auto cb : g_InputCallbacks) {
      cb(uMsg, wParam);
    }
  }

  return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT APIENTRY hkReset(IDirect3DDevice9 *pDevice,
                         D3DPRESENT_PARAMETERS *pPresentationParameters) {
  if (g_ImGuiInitialized) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
  }

  HRESULT result = oReset(pDevice, pPresentationParameters);

  if (g_ImGuiInitialized && SUCCEEDED(result)) {
    ImGui_ImplDX9_CreateDeviceObjects();
  }

  return result;
}

void __fastcall hkGameLoop(void *ecx, void *edx, int32_t *arg1) {
  for (auto cb : g_TickCallbacks) {
    cb();
  }
  oGameLoop(ecx, edx, arg1);
}

HRESULT APIENTRY hkPresent(IDirect3DDevice9 *pDevice, const RECT *pSourceRect,
                           const RECT *pDestRect, HWND hDestWindowOverride,
                           const RGNDATA *pDirtyRegion) {
  if (!g_ImGuiInitialized) {
    D3DDEVICE_CREATION_PARAMETERS params;
    pDevice->GetCreationParameters(&params);

    HWND hWindow = params.hFocusWindow;
    if (!hWindow)
      hWindow = GetForegroundWindow();

    oWndProc =
        (WNDPROC)SetWindowLongPtr(hWindow, GWL_WNDPROC, (LONG_PTR)WndProc);

    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hWindow);
    ImGui_ImplDX9_Init(pDevice);
    g_ImGuiInitialized = true;

    InitLuaEngine();
  }

  IDirect3DStateBlock9 *stateBlock = nullptr;
  if (pDevice->CreateStateBlock(D3DSBT_ALL, &stateBlock) == D3D_OK) {
    stateBlock->Capture();
  }

  ImGui_ImplDX9_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  DrawImGuiMenu();

  ImGui::EndFrame();
  ImGui::Render();

  if (pDevice->BeginScene() == D3D_OK) {
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
      CW_USEDEFAULT, 300, 300, NULL, NULL, NULL, NULL);

  IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
  if (!d3d) {
    DestroyWindow(dummyWindow);
    return;
  }

  D3DPRESENT_PARAMETERS d3dpp = {};
  d3dpp.Windowed = TRUE;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.hDeviceWindow = dummyWindow;

  IDirect3DDevice9 *dummyDevice = nullptr;
  if (SUCCEEDED(d3d->CreateDevice(
          D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, dummyWindow,
          D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dummyDevice))) {
    void **vTable = *reinterpret_cast<void ***>(dummyDevice);

    void *presentAddress = vTable[17];
    void *resetAddress = vTable[16];
    void *updateSceneAddress = (void *)0x0075D8C0;

    MH_CreateHook(presentAddress, (void *)hkPresent, (void **)&oPresent);
    MH_CreateHook(resetAddress, (void *)hkReset, (void **)&oReset);
    MH_CreateHook(updateSceneAddress, (void *)hkGameLoop, (void **)&oGameLoop);

    MH_EnableHook(MH_ALL_HOOKS);

    dummyDevice->Release();
  }

  d3d->Release();
  DestroyWindow(dummyWindow);
}
