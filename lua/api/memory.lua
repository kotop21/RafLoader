local ffi = require("ffi")

return {
  -- Забивает NOP-ами (0x90)
  write_nop = function(address, size)
    print(string.format("[Memory] NOP patch: 0x%X (Size: %d)", address, size))
    local nops = ffi.new("uint8_t[?]", size)
    ffi.fill(nops, size, 0x90)
    return _G.Core.Core_PatchMemory(address, nops, size)
  end,

  -- Пишет произвольные байты (например: {0xEB, 0x01})
  patch = function(address, bytes)
    print(string.format("[Memory] Byte patch: 0x%X (Bytes: %d)", address, #bytes))
    local b = ffi.new("uint8_t[?]", #bytes, bytes)
    return _G.Core.Core_PatchMemory(address, b, #bytes)
  end,

  -- Чтение значений
  read_int = function(addr)
    return ffi.cast("int32_t*", addr)[0]
  end,

  read_float = function(addr)
    return ffi.cast("float*", addr)[0]
  end,

  -- Запись значений
  write_int = function(addr, val)
    print(string.format("[Memory] Write Int32: %d -> 0x%X", val, addr))
    ffi.cast("int32_t*", addr)[0] = val
  end,

  write_float = function(addr, val)
    print(string.format("[Memory] Write Float: %.2f -> 0x%X", val, addr))
    ffi.cast("float*", addr)[0] = val
  end
}
