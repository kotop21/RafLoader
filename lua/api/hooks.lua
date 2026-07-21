local ffi = require("ffi")

_G.ActiveHooks = _G.ActiveHooks or {}
_G.ActiveCallbacks = _G.ActiveCallbacks or {}

return {
	create = function(address, signature, callback)
		assert(type(signature) == "string", "signature must be a string")
		assert(type(callback) == "function", "callback must be a function")

		local type_name = "Hook_" .. string.format("%X", address)
		local cdef_str = "typedef " .. signature:gsub("NAME", type_name) .. ";"

		ffi.cdef(cdef_str)

		local detour = ffi.cast(type_name, callback)
		_G.ActiveHooks[type_name] = detour

		local orig = _G.Core.Core_CreateHook(ffi.cast("void*", address), ffi.cast("void*", detour))

		if orig == nil then
			print(string.format("[Hooks] Failed to hook 0x%X", address))
			return nil
		end

		print(string.format("[Hooks] Hook installed at 0x%X", address))

		return ffi.cast(type_name, orig)
	end,

	create_usercall = function(address, callback)
		assert(type(callback) == "function", "callback must be a function")

		local cb = ffi.cast("int (__cdecl *)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t)", callback)

		_G.ActiveCallbacks[tostring(address)] = cb

		local orig = _G.Core.CreateUsercallBridge(ffi.cast("void*", address), cb)

		if orig == nil then
			print(string.format("[Hooks] Failed to create usercall bridge at 0x%X", address))
		end

		return orig
	end,

	create_damage_hook = function(address, callback)
		assert(type(callback) == "function", "callback must be a function")

		local cb = ffi.cast("void (__cdecl *)(void*, int, int, int, int)", callback)

		_G.ActiveCallbacks["DamagePostHook"] = cb

		local orig = _G.Core.CreateDamagePostHook(ffi.cast("void*", address), cb)

		if orig == nil then
			print(string.format("[Hooks] Failed to create damage hook at 0x%X", address))
		end

		return orig
	end,
}
