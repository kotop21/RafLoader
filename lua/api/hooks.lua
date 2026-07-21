local ffi = require("ffi")

_G.ActiveHooks = _G.ActiveHooks or {}
_G.ActiveCallbacks = _G.ActiveCallbacks or {}

return {
  create = function(address, signature, callback)
    local type_name = "Hook_" .. string.format("%X", address)
    local cdef_str = "typedef " .. string.gsub(signature, "NAME", type_name) .. ";"
    ffi.cdef(cdef_str)

    local detour = ffi.cast(type_name, callback)
    _G.ActiveHooks[type_name] = detour

    local orig = _G.Core.Core_CreateHook(ffi.cast("void*", address), ffi.cast("void*", detour))
    if orig ~= nil then
      return ffi.cast(type_name, orig)
    end
    return nil
  end,

  create_usercall = function(address, callback)
    local cb = ffi.cast("int (__cdecl *)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t)", callback)
    _G.ActiveCallbacks[tostring(address)] = cb

    local orig = _G.Core.CreateUsercallBridge(ffi.cast("void*", address), cb)
    if orig ~= nil then
      return orig
    end
    return nil
  end,

  create_damage_hook = function(address, callback)
    local cb = ffi.cast("void (__cdecl *)(void*, int, int, int, int)", callback)
    _G.ActiveCallbacks["DamagePostHook"] = cb

    local orig = _G.Core.CreateDamagePostHook(ffi.cast("void*", address), cb)
    if orig ~= nil then
      return orig
    end
    return nil
  end
}
