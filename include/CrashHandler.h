#pragma once
#include <stdint.h>
#include <windows.h>

void SetupVectoredRecovery();

/**
 * Регистрирует точку восстановления.
 * @param faultAddr    Адрес инструкции в игре, которая вызывает крэш (например,
 * обращение к NULL).
 * @param recoveryAddr Адрес (EIP), на который нужно принудительно прыгнуть,
 * чтобы продолжить работу.
 */
extern "C" __declspec(dllexport) void __cdecl
Core_RegisterRecoveryPoint(uintptr_t faultAddr, uintptr_t recoveryAddr);
