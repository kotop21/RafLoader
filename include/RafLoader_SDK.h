#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Здесь __declspec(dllimport), потому что плагин ИМПОРТИРУЕТ
// это из RafLoader.asi
__declspec(dllimport) void __cdecl Core_Log(const char *text);
__declspec(dllimport) void __cdecl Core_ToggleConsole(bool state);
__declspec(dllimport) bool Core_UnprotectMemory(uintptr_t address, size_t size);
__declspec(dllimport) bool
Core_PatchMemory(uintptr_t address, const uint8_t *newBytes, size_t size);
__declspec(dllimport) void *Core_CreateHook(void *target, void *detour);
__declspec(dllimport) void __cdecl
Core_RegisterRecoveryPoint(uintptr_t faultAddr, uintptr_t recoveryAddr);

#ifdef __cplusplus
}
#endif
