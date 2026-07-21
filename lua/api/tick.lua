local ffi = require("ffi")

local callbacks = {}
local tick_cb_anchor

tick_cb_anchor = ffi.cast("void (*)(void)", function()
	for i = 1, #callbacks do
		local ok, err = pcall(callbacks[i])

		if not ok then
			print("[Tick] Callback error:")
			print(err)
		end
	end
end)

_G.TickCallback_KeepAlive = tick_cb_anchor
_G.Core.Core_RegisterTickCallback(tick_cb_anchor)

return {
	add = function(callback_fn)
		assert(type(callback_fn) == "function", "callback_fn must be a function")

		callbacks[#callbacks + 1] = callback_fn

		print(string.format("[Tick] Callback registered. Total: %d", #callbacks))
	end,

	clear = function()
		callbacks = {}
		print("[Tick] All callbacks cleared.")
	end,
}
