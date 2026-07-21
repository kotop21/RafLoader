local ffi = require("ffi")

ffi.cdef [[
  short GetKeyState(int nVirtKey);
]]

local binds = {}
local WM_KEYDOWN = 0x0100

local keyMap = {
  ["F1"] = 0x70,
  ["F2"] = 0x71,
  ["F3"] = 0x72,
  ["F4"] = 0x73,
  ["F5"] = 0x74,
  ["F6"] = 0x75,
  ["F7"] = 0x76,
  ["F8"] = 0x77,
  ["F9"] = 0x78,
  ["F10"] = 0x79,
  ["F11"] = 0x7A,
  ["F12"] = 0x7B,
  ["SPACE"] = 0x20,
  ["ENTER"] = 0x0D,
  ["ESC"] = 0x1B,
  ["TAB"] = 0x09,
  ["SHIFT"] = 0x10,
  ["CTRL"] = 0x11,
  ["ALT"] = 0x12
}

local function resolveKey(key)
  if type(key) == "number" then
    return key, false
  elseif type(key) == "string" then
    if #key == 1 and key:match("%a") then
      return string.byte(key:upper()), key == key:upper()
    elseif #key == 1 then
      return string.byte(key:upper()), false
    end
    return keyMap[key:upper()], false
  end
  return nil, false
end

local inputCallback = ffi.cast("void (*)(uint32_t, uintptr_t)", function(uMsg, wParam)
  if uMsg == WM_KEYDOWN then
    local key = tonumber(wParam)
    if binds[key] then
      local isShift = bit.band(ffi.C.GetKeyState(0x10), 0x8000) ~= 0
      if isShift and binds[key].shift then
        binds[key].shift()
      elseif not isShift and binds[key].plain then
        binds[key].plain()
      end
    end
  end
end)

_G.InputCallback_KeepAlive = inputCallback
_G.Core.Core_RegisterInputCallback(inputCallback)

return {
  bind = function(key, callback)
    local vkCode, reqShift = resolveKey(key)
    if vkCode then
      binds[vkCode] = binds[vkCode] or {}
      if reqShift then
        binds[vkCode].shift = callback
      else
        binds[vkCode].plain = callback
      end
    end
  end,

  unbind = function(key)
    local vkCode, reqShift = resolveKey(key)
    if vkCode and binds[vkCode] then
      if reqShift then
        binds[vkCode].shift = nil
      else
        binds[vkCode].plain = nil
      end
    end
  end
}
