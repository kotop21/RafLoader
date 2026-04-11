#include "CrashHandler.h"
// #include "RenderHook.h"
#include "thread.h"
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);

    // InitRenderHook();
    SetupVectoredRecovery();

    CreateThread(nullptr, 0, ModThread, hModule, 0, nullptr);
  }
  return TRUE;
}
