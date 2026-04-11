local ffi = require("ffi")

return {
  catch = function(crash_addr, safe_addr)
    print(string.format("[CrashHandler] Protection established: 0x%X -> jump to 0x%X", crash_addr, safe_addr))

    _G.Core.Core_RegisterRecoveryPoint(crash_addr, safe_addr)
  end
}
