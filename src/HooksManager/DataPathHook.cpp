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
      "push [esp + 48]\n"
      "push [esp + 48]\n"
      "push [esp + 48]\n"
      "push esi\n"
      "push edx\n"
      "push ecx\n"
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

void *g_LuaDamageCallback = nullptr;
void *g_OrigTakeDamage = nullptr;

#ifdef _MSC_VER
#define CC_THISCALL __thiscall
#else
#define CC_THISCALL __attribute__((thiscall))
#endif

void *CC_THISCALL hkTakeDamage_Post(void *pThis, int attacker, int raw_dmg,
                                    void *arg4, int pad4, int pad5, int pad6,
                                    int pad7) {
  int hp_before = 0;
  if (pThis)
    hp_before = *(int *)((char *)pThis + 0xB8);

  typedef void *(CC_THISCALL * OrigFn_t)(void *, int, int, void *, int, int,
                                         int, int);
  void *result = ((OrigFn_t)g_OrigTakeDamage)(pThis, attacker, raw_dmg, arg4,
                                              pad4, pad5, pad6, pad7);

  int hp_after = 0;
  if (pThis)
    hp_after = *(int *)((char *)pThis + 0xB8);

  if (g_LuaDamageCallback && pThis && attacker != 0 && raw_dmg > 0) {
    typedef void(__cdecl * LuaCb_t)(void *, int, int, int, int);
    ((LuaCb_t)g_LuaDamageCallback)(pThis, attacker, raw_dmg, hp_before,
                                   hp_after);
  }

  return result;
}

extern "C" __declspec(dllexport) void *CreateDamagePostHook(void *targetAddress,
                                                            void *luaCallback) {
  g_LuaDamageCallback = luaCallback;
  if (MH_CreateHook(targetAddress, (void *)hkTakeDamage_Post,
                    &g_OrigTakeDamage) == MH_OK) {
    MH_EnableHook(targetAddress);
    return g_OrigTakeDamage;
  }
  return nullptr;
}
