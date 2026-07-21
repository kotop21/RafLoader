#include "ConsoleState.h"
#include <MinHook.h>
#include <stdio.h>
#include <windows.h>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include "lua/ScriptsLoader_lua.h"
#include "lua/VahCrash_lua.h"
#include "lua/core_lua.h"
#include "lua/hooks_lua.h"
#include "lua/memory_lua.h"
#include "lua/tick_lua.h"

extern "C" void Core_Log(const char *text);

extern void InitRenderHook();

DWORD WINAPI InitHooksThread(LPVOID lpParam) {
  MH_STATUS status = MH_Initialize();

  if (status != MH_OK) {
    printf("[MinHook] Initialization failed (%d)\n", status);
    return 0;
  }

  InitRenderHook();

  return 0;
}

lua_State *G_LuaState = nullptr;

void PreloadEmbeddedModule(lua_State *L, const char *moduleName,
                           const unsigned char *bytecode, size_t size) {
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "preload");

  if (luaL_loadbuffer(L, (const char *)bytecode, size, moduleName) == LUA_OK) {
    lua_setfield(L, -2, moduleName);
  } else {
    printf("[Lua] Preload error %s: %s\n", moduleName, lua_tostring(L, -1));
    lua_pop(L, 1);
  }

  lua_pop(L, 2);
}

void InitLuaEngine() {
  if (G_LuaState != nullptr)
    return;

  SetConsoleOutputCP(CP_UTF8);

  FILE *fDummy = nullptr;

  if (freopen_s(&fDummy, "CONOUT$", "w", stdout) != 0)
    printf("[RafLoader] Failed to redirect stdout.\n");

  if (freopen_s(&fDummy, "CONOUT$", "w", stderr) != 0)
    printf("[RafLoader] Failed to redirect stderr.\n");

  remove("RafLoader.log");

  printf("[RafLoader v%s] Initializing Lua on Main Thread...\n", RAF_VERSION);

  lua_State *L = luaL_newstate();

  if (!L) {
    printf("[Lua] Failed to create Lua state.\n");
    return;
  }

  luaL_openlibs(L);

  PreloadEmbeddedModule(L, "memory", memory_lua, memory_lua_SIZE);
  PreloadEmbeddedModule(L, "hooks", hooks_lua, hooks_lua_SIZE);
  PreloadEmbeddedModule(L, "VahCrash", VahCrash_lua, VahCrash_lua_SIZE);
  PreloadEmbeddedModule(L, "ScriptsLoader", ScriptsLoader_lua,
                        ScriptsLoader_lua_SIZE);
  PreloadEmbeddedModule(L, "tick", tick_lua, tick_lua_SIZE);

  printf("[RafLoader v%s] LuaJIT is ready.\n", RAF_VERSION);

  if (luaL_loadbuffer(L, (const char *)core_lua, core_lua_SIZE,
                      "core_embedded") != LUA_OK ||
      lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
    printf("[Lua Core Error] %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    lua_close(L);
    return;
  }

  printf("[RafLoader] Core started successfully!\n");

  G_LuaState = L;
}
