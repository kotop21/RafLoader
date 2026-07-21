#include <map>
#include <stdint.h>
#include <windows.h>

std::map<uintptr_t, uintptr_t> RecoveryPoints;

LONG WINAPI VehRecoveryHandler(EXCEPTION_POINTERS *ExceptionInfo) {
  if (ExceptionInfo->ExceptionRecord->ExceptionCode ==
      EXCEPTION_ACCESS_VIOLATION) {

    uintptr_t faultAddress =
        (uintptr_t)ExceptionInfo->ExceptionRecord->ExceptionAddress;

    auto it = RecoveryPoints.find(faultAddress);
    if (it != RecoveryPoints.end()) {
      ExceptionInfo->ContextRecord->Eip = it->second;

      return EXCEPTION_CONTINUE_EXECUTION;
    }
  }

  // Если ничего не передано игра вылетит
  return EXCEPTION_CONTINUE_SEARCH;
}

void SetupVectoredRecovery() {
  AddVectoredExceptionHandler(1, VehRecoveryHandler);
}

extern "C" __declspec(dllexport) void __cdecl
Core_RegisterRecoveryPoint(uintptr_t faultAddr, uintptr_t recoveryAddr) {
  RecoveryPoints[faultAddr] = recoveryAddr;
}
