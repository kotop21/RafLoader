#include "DefaultHooks.h"
#include "ConsoleState.h"
#include <MinHook.h>
#include <stdio.h>

extern "C" __declspec(dllexport) void *Core_CreateHook(void *target,
                                                       void *detour) {
  void *original = nullptr;

  // Создаем хук в памяти
  if (MH_CreateHook(target, detour, &original) != MH_OK) {
    printf("[Core_Hook] Error creating hook at address %p\n", target);
    return nullptr;
  }

  // Активируем его
  if (MH_EnableHook(target) != MH_OK) {
    printf("[Core_Hook] Error enabling hook at address %p\n", target);
    return nullptr;
  }

  return original;
}
