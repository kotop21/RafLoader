# Документация по API ядра RafLoader (LuaJIT)

Этот документ описывает использование функций C++ для работы с памятью и хуками через Lua.

## Инициализация и Логирование
Загрузите плагин и переопределите `print` для вывода в консоль плагина.

```lua
local ffi = require("ffi")
ffi.cdef[[
    void Core_Log(const char *text);
    bool __cdecl Core_PatchMemory(uintptr_t address, const uint8_t* newBytes, size_t size);
    void* __cdecl Core_CreateHook(void* target, void* detour);
]]
local raf = ffi.load("RafLoader_v0.0.1.asi")
_G.print = function(...)
    local args, str = {...}, {}
    for i, v in ipairs(args) do table.insert(str, tostring(v)) end
    raf.Core_Log(table.concat(str, " "))
end
print(">>> [Lua] Инициализировано.")```

## Работа с памятью (Core_PatchMemory)
Безопасно пишет байты, автоматически управляя VirtualProtect.
```lua
local function NopMemory(address, size)
  local nops = ffi.new("uint8_t[?]", size)
  ffi.fill(nops, size, 0x90) 
  if raf.Core_PatchMemory(address, nops, size) then
      print(string.format("Noped 0x%X", address))
  end
end
NopMemory(0x00872A30, 16)
```

## Перехват функций (Hooks)

Использует MinHook. Важно: сохраняйте колбеки в _G.ActiveHooks, чтобы их не удалил Garbage Collector (GC).

```lua
ffi.cdef[[ typedef int32_t (__cdecl *Lobby_LoadSettings_t)(); ]]
_G.ActiveHooks = _G.ActiveHooks or {}
local target, original = ffi.cast("void*", 0x00924350), nil

local detour = ffi.cast("Lobby_LoadSettings_t", function()
    print("[Hook] Lobby Triggered")
    return original ~= nil and ffi.cast("Lobby_LoadSettings_t", original)() or 0
end)

table.insert(_G.ActiveHooks, detour)
original = raf.Core_CreateHook(target, ffi.cast("void*", detour))
```

## 4. Фоновые потоки (Мониторинг)

Для цикличной проверки памяти без фризов игры.

```lua
ffi.cdef[[
    typedef unsigned long (__stdcall *PTHREAD_START_ROUTINE)(void*);
    void* CreateThread(void*, size_t, PTHREAD_START_ROUTINE, void*, unsigned long, unsigned long*);
    void Sleep(unsigned long ms);
]]
local kernel32 = ffi.load("kernel32")
local threadFunc = ffi.cast("PTHREAD_START_ROUTINE", function()
    local ptr = ffi.cast("int32_t*", 0x101E794)
    while true do
        print("PopLimit:", ptr[0])
        kernel32.Sleep(1000)
    end
end)
table.insert(_G.ActiveHooks, threadFunc)
kernel32.CreateThread(nil, 0, threadFunc, nil, 0, nil)
```
