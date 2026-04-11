#include "memory.h"
#include <string.h>

extern "C" __declspec(dllexport) bool Core_UnprotectMemory(uintptr_t address,
                                                           size_t size) {
  DWORD oldProtect;
  return VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE,
                        &oldProtect) != 0;
}

extern "C" __declspec(dllexport) bool
Core_PatchMemory(uintptr_t address, const uint8_t *newBytes, size_t size) {
  DWORD oldProtect;

  if (VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE,
                     &oldProtect)) {
    memcpy((void *)address, newBytes, size);

    VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);
    return true;
  }
  return false;
}
