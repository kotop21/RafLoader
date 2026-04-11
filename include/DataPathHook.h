#pragma once
#include <windows.h>

extern "C" {
__declspec(dllexport) void *CreateUsercallBridge(void *targetAddress,
                                                 void *luaCallback);
}
