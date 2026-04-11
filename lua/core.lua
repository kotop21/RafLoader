local ffi = require("ffi")

ffi.cdef [[
    typedef struct {
        uint32_t dwFileAttributes;
        uint32_t ftCreationTime[2];
        uint32_t ftLastAccessTime[2];
        uint32_t ftLastWriteTime[2];
        uint32_t nFileSizeHigh;
        uint32_t nFileSizeLow;
        uint32_t dwReserved0;
        uint32_t dwReserved1;
        char     cFileName[260];
        char     cAlternateFileName[14];
    } WIN32_FIND_DATAA;

    void* FindFirstFileA(const char* lpFileName, WIN32_FIND_DATAA* lpFindFileData);
    bool  FindNextFileA(void* hFindFile, WIN32_FIND_DATAA* lpFindFileData);
    bool  FindClose(void* hFindFile);

    void Core_Log(const char *text);
    bool __cdecl Core_PatchMemory(uintptr_t address, const uint8_t* newBytes, size_t size);
    void* __cdecl Core_CreateHook(void* target, void* detour);
    void* CreateUsercallBridge(void* targetAddress, void* luaCallback);
    void __cdecl Core_RegisterRecoveryPoint(uintptr_t crashAddr, uintptr_t safeAddr);

    void __cdecl Core_RegisterTickCallback(void* cb);
]]

_G.Core        = ffi.load("RafLoader.asi")
package.path   = "./scripts/?.lua;?.lua"

_G.ActiveHooks = {}
_G.Engine      = {}

_G.print       = function(...)
    local args = { ... }
    local str = {}
    for i, v in ipairs(args) do table.insert(str, tostring(v)) end
    _G.Core.Core_Log(table.concat(str, "\t"))
end


-- API
_G.Engine.Memory   = require("memory")
_G.Engine.Hooks    = require("hooks")
_G.Engine.VahCrash = require("VahCrash")
_G.Engine.Input    = require("input")

local loader       = require("ScriptsLoader")

print("[Core] System ready. API composed. \n")
loader("scripts")
