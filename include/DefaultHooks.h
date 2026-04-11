#pragma once
#include <windows.h>

extern "C" {
__declspec(dllexport) void *Core_CreateHook(void *target, void *detour);
}
