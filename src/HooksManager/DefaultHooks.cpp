#include "DefaultHooks.h"
#include "ConsoleState.h"

#include <MinHook.h>
#include <stdio.h>

extern "C" __declspec(dllexport) void *Core_CreateHook(void *target,
                                                       void *detour) {
  if (!target || !detour) {
    printf("[Core_Hook] Invalid target (%p) or detour (%p)\n", target, detour);
    return nullptr;
  }

  void *original = nullptr;

  MH_STATUS status = MH_CreateHook(target, detour, &original);
  if (status != MH_OK) {
    printf("[Core_Hook] MH_CreateHook failed (%d) at %p\n", status, target);
    return nullptr;
  }

  status = MH_EnableHook(target);
  if (status != MH_OK) {
    MH_RemoveHook(target);

    printf("[Core_Hook] MH_EnableHook failed (%d) at %p\n", status, target);
    return nullptr;
  }

  return original;
}
