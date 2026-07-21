#include "CrashHandler.h"
#include <windows.h>

extern DWORD WINAPI InitHooksThread(LPVOID lpParam);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    SetupVectoredRecovery();

    HANDLE hThread =
        CreateThread(nullptr, 0, InitHooksThread, hModule, 0, nullptr);

    if (hThread) {
      CloseHandle(hThread);
    }
  }

  return TRUE;
}
