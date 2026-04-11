#pragma once
#include <stddef.h>
#include <stdint.h>
#include <windows.h>

extern "C" {
// Просто снимает защиту (PAGE_EXECUTE_READWRITE)
__declspec(dllexport) bool Core_UnprotectMemory(uintptr_t address, size_t size);

// Безопасно записывает байты (Снимает защиту -> Пишет -> Возвращает старую
// защиту)
__declspec(dllexport) bool
Core_PatchMemory(uintptr_t address, const uint8_t *newBytes, size_t size);
}
