local ffi = require("ffi")

local callbacks = {}
local tick_cb_anchor

tick_cb_anchor = ffi.cast("void (*)(void)", function()
  for i = 1, #callbacks do
    callbacks[i]()
  end
end)

_G.TickCallback_KeepAlive = tick_cb_anchor
_G.Core.Core_RegisterTickCallback(tick_cb_anchor)

return {
  add = function(callback_fn)
    table.insert(callbacks, callback_fn)
    print(string.format("[Tick] Callback registered. Total: %d", #callbacks))
  end,

  clear = function()
    callbacks = {}
    print("[Tick] All callbacks cleared.")
  end
}
