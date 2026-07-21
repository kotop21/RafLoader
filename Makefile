VERSION = 0.2.1

BUILD_DIR = build

CXX = i686-w64-mingw32-g++

CXXFLAGS = -m32 -std=c++17 -O2 -shared -s \
           -ffunction-sections -fdata-sections \
           -DRAF_VERSION=\"$(VERSION)\" \
           -I./include -I./include/lua -I./lib/luajit -I./lib/minhook -I./lib/imgui

LDFLAGS = -static -static-libgcc -static-libstdc++ \
          -Wl,--gc-sections \
          ./lib/luajit/libluajit.a \
          -Wl,--subsystem,windows:5.1 \
          -luser32 -lkernel32 -lwinmm -ld3d9 -lgdi32 -ldwmapi

MINHOOK_SRC = lib/minhook/src/buffer.c \
              lib/minhook/src/hook.c \
              lib/minhook/src/trampoline.c \
              lib/minhook/src/hde/hde32.c

IMGUI_SRC = lib/imgui/imgui.cpp \
            lib/imgui/imgui_draw.cpp \
            lib/imgui/imgui_widgets.cpp \
            lib/imgui/imgui_tables.cpp \
            lib/imgui/imgui_impl_dx9.cpp \
            lib/imgui/imgui_impl_win32.cpp

SRC = src/main.cpp src/thread.cpp src/memory.cpp src/HooksManager/DefaultHooks.cpp src/CrashHandler.cpp src/ConsoleState.cpp src/ImGuiMenu.cpp src/RenderHook.cpp $(IMGUI_SRC) $(MINHOOK_SRC)

OUT = $(BUILD_DIR)/RafLoader.asi

LUAJIT = ./lib/luajit/luajit

LUA_SOURCES = $(wildcard lua/*.lua) $(wildcard lua/api/*.lua)
LUA_HEADERS = $(patsubst %.lua, include/lua/%_lua.h, $(notdir $(LUA_SOURCES)))

all: $(LUA_HEADERS) $(OUT)
	@echo ""
	@echo "\033[1;32mBUILD SUCCESSFUL: $(OUT) v$(VERSION)\033[0m"
	@echo ""

include/lua/%_lua.h: lua/api/%.lua
	mkdir -p include/lua
	$(LUAJIT) builder.lua $< $@ $*_lua

include/lua/%_lua.h: lua/%.lua
	mkdir -p include/lua
	$(LUAJIT) builder.lua $< $@ $*_lua

$(OUT): $(SRC) $(LUA_HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS) -Wl,--out-implib,$(BUILD_DIR)/libRafLoader.a
	cp include/RafLoader_SDK.h $(BUILD_DIR)/RafLoader_SDK.h

clean:
	rm -rf $(BUILD_DIR)
	rm -rf include/lua
