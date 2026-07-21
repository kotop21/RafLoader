#include <map>
#include <mutex>
#include <stdint.h>
#include <windows.h>

std::map<uintptr_t, uintptr_t> RecoveryPoints;
std::mutex RecoveryMutex;

LONG WINAPI VehRecoveryHandler(EXCEPTION_POINTERS *ExceptionInfo) {
  if (ExceptionInfo->ExceptionRecord->ExceptionCode ==
      EXCEPTION_ACCESS_VIOLATION) {

    uintptr_t faultAddress =
        (uintptr_t)ExceptionInfo->ExceptionRecord->ExceptionAddress;

    std::lock_guard<std::mutex> lock(RecoveryMutex);

    auto it = RecoveryPoints.find(faultAddress);
    if (it != RecoveryPoints.end()) {
      ExceptionInfo->ContextRecord->Eip = it->second;
      return EXCEPTION_CONTINUE_EXECUTION;
    }
  }

  // Если ничего не найдено — передаем обработку дальше.
  return EXCEPTION_CONTINUE_SEARCH;
}

void SetupVectoredRecovery() {
  if (!AddVectoredExceptionHandler(1, VehRecoveryHandler)) {
    OutputDebugStringA(
        "[RafLoader] Failed to register vectored exception handler.\n");
  }
}

extern "C" __declspec(dllexport) void __cdecl
Core_RegisterRecoveryPoint(uintptr_t faultAddr, uintptr_t recoveryAddr) {
  std::lock_guard<std::mutex> lock(RecoveryMutex);
  RecoveryPoints[faultAddr] = recoveryAddr;
}
