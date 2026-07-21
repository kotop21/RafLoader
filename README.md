<div align="center">
  <h1>⚔️ RafLoader (Rise and Fall: Civilization at War)</h1>
</div>

RafLoader is a lightweight ASI loader for **Rise and Fall: Civilization at War**, providing runtime modding through **LuaJIT** and native **C++** APIs.

---

## 🚀 Overview

RafLoader injects into the game process and provides:

- Native function hooking powered by **MinHook**.
- Embedded LuaJIT runtime.
- DirectX9 rendering hook.
- Automatic Lua script loading.
- Built-in ImGui debug console.
- Crash recovery helpers.
- Per-frame (tick) callbacks.

---

# 🧩 Features

## 🪝 Function Hooks

Create native hooks directly from Lua.

Supported hook types:

- Standard function hooks.
- `__usercall` bridge hooks.
- Damage post hooks.

Example:

```lua
local hooks = _G.Engine.Hooks

local original

original = hooks.create(
    0x401000,
    "int (__cdecl *NAME)(int)",
    function(value)
        print("Hook:", value)
        return original(value)
    end
)
```

---

## 💾 Memory API

Read and modify game memory.

Available functions:

```lua
local memory = _G.Engine.Memory

memory.write_nop(address, size)
memory.patch(address, bytes)

memory.read_int(address)
memory.read_float(address)

memory.write_int(address, value)
memory.write_float(address, value)
```

Example:

```lua
memory.write_nop(0x401000, 5)

memory.patch(0x401100, {
    0x90,
    0x90,
    0x90
})
```

---

## 🔁 Tick Callbacks

Execute Lua code every game update.

```lua
local tick = _G.Engine.Tick

tick.add(function()
    -- Called every frame
end)
```

Clear all callbacks:

```lua
tick.clear()
```

---

## 🛡️ Crash Recovery

Register recovery points for known crashes.

```lua
local crash = _G.Engine.VahCrash

crash.catch(
    0x403000,
    0x404000
)
```

---

## 📂 Script Loader

All Lua scripts inside the `scripts` directory are loaded automatically.

Example structure:

```
RafLoader.asi
scripts/
    test.lua
    cheats.lua
    ui.lua
```

Files beginning with `_` or `.` are ignored.

---

## 🎨 Debug Console

Built-in ImGui console.

Features:

- Live log output.
- Automatic logging from Lua `print()`.
- Log file (`RafLoader.log`).
- Toggle with **F1**.

---

# 🛠️ Complete Example

```lua
local memory = _G.Engine.Memory
local hooks  = _G.Engine.Hooks
local tick   = _G.Engine.Tick
local crash  = _G.Engine.VahCrash

memory.write_nop(0x401000, 5)

local original

original = hooks.create(
    0x401050,
    "int (__cdecl *NAME)(int)",
    function(value)
        print("Function called:", value)
        return original(value)
    end
)

tick.add(function()
    -- Executed every frame
end)

crash.catch(
    0x403000,
    0x404000
)
```

---

# 🧱 Extending RafLoader

The Lua API is exposed through:

```lua
_G.Engine.Memory
_G.Engine.Hooks
_G.Engine.Tick
_G.Engine.VahCrash
```

Additional native functionality can be implemented through custom C++ plugins.

---

# ⚠️ Limitations

- x86 only.
- DirectX9 renderer.
- Requires knowledge of game memory addresses.
- Invalid hooks or memory patches may crash the game.

---

RafLoader provides a compact and lightweight framework for creating Lua-powered mods, runtime patches, hooks, and debugging tools for **Rise and Fall: Civilization at War**.
