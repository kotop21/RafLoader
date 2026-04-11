local ffi = require("ffi")

ffi.cdef [[
    short GetAsyncKeyState(int vKey);
]]

local binds = {}
local states = {}

local function on_tick()
  for key, cb in pairs(binds) do
    local state = bit.band(ffi.C.GetAsyncKeyState(key), 0x8000) ~= 0
    if state and not states[key] then
      cb()
    end
    states[key] = state
  end
end

_G._CoreTickCallback = ffi.cast("void (*)()", on_tick)
_G.Core.Core_RegisterTickCallback(_G._CoreTickCallback)

return {
  bind = function(key, callback)
    binds[key] = callback
    states[key] = false
  end,
  unbind = function(key)
    binds[key] = nil
    states[key] = nil
  end
}
