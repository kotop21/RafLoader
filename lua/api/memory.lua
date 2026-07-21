local ffi = require("ffi")

return {

	write_nop = function(address, size)
		assert(size > 0, "size must be > 0")

		print(string.format("[Memory] NOP patch: 0x%X (%d bytes)", address, size))

		local nops = ffi.new("uint8_t[?]", size)
		ffi.fill(nops, size, 0x90)

		return _G.Core.Core_PatchMemory(address, nops, size)
	end,

	patch = function(address, bytes)
		assert(type(bytes) == "table", "bytes must be a table")
		assert(#bytes > 0, "bytes cannot be empty")

		print(string.format("[Memory] Byte patch: 0x%X (%d bytes)", address, #bytes))

		local buffer = ffi.new("uint8_t[?]", #bytes)

		for i = 1, #bytes do
			buffer[i - 1] = bytes[i]
		end

		return _G.Core.Core_PatchMemory(address, buffer, #bytes)
	end,

	read_int = function(addr)
		return ffi.cast("int32_t*", addr)[0]
	end,

	read_float = function(addr)
		return ffi.cast("float*", addr)[0]
	end,

	write_int = function(addr, val)
		print(string.format("[Memory] Write Int32: %d -> 0x%X", val, addr))

		ffi.cast("int32_t*", addr)[0] = val
	end,

	write_float = function(addr, val)
		print(string.format("[Memory] Write Float: %.2f -> 0x%X", val, addr))

		ffi.cast("float*", addr)[0] = val
	end,
}
