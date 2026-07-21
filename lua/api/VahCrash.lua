local ffi = require("ffi")

return {
	catch = function(crash_addr, safe_addr)
		print(string.format("[CrashHandler] Recovery: 0x%X -> 0x%X", crash_addr, safe_addr))

		_G.Core.Core_RegisterRecoveryPoint(crash_addr, safe_addr)
	end,
}
