#include "memory.h"
#include <string.h>
#include <windows.h>

extern "C" __declspec(dllexport) bool Core_UnprotectMemory(uintptr_t address,
                                                           size_t size) {
  DWORD oldProtect;
  return VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE,
                        &oldProtect) != 0;
}

extern "C" __declspec(dllexport) bool
Core_PatchMemory(uintptr_t address, const uint8_t *newBytes, size_t size) {
  if (!newBytes || size == 0)
    return false;

  DWORD oldProtect;

  if (!VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE,
                      &oldProtect))
    return false;

  memcpy((void *)address, newBytes, size);

  FlushInstructionCache(GetCurrentProcess(), (LPCVOID)address, size);

  VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);

  return true;
}
