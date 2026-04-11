#include "DataPathHook.h"
#include <MinHook.h>

void *g_LuaCallback = nullptr;
void *g_OriginalFunc = nullptr;

__attribute__((naked)) void hkUsercallBridge_Naked() {
  asm(".intel_syntax noprefix\n"
      "pushad\n"
      "pushfd\n"

      "mov eax, _g_LuaCallback\n"
      "test eax, eax\n"
      "jz skip_lua\n"

      "mov ebx, [esp + 44]\n"
      "mov edi, [esp + 40]\n"

      "push ebx\n"
      "push edi\n"
      "push esi\n"
      "push ecx\n"
      "push edx\n"
      "push eax\n"

      "call eax\n"
      "add esp, 24\n"

      "skip_lua:\n"
      "popfd\n"
      "popad\n"
      "jmp [_g_OriginalFunc]\n"
      ".att_syntax\n");
}

extern "C" __declspec(dllexport) void *CreateUsercallBridge(void *targetAddress,
                                                            void *luaCallback) {
  g_LuaCallback = luaCallback;

  if (MH_CreateHook(targetAddress, (LPVOID)hkUsercallBridge_Naked,
                    &g_OriginalFunc) != MH_OK) {
    return nullptr;
  }

  if (MH_EnableHook(targetAddress) != MH_OK) {
    return nullptr;
  }

  return g_OriginalFunc;
}
