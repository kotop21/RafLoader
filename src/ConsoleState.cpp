#define LOG_SYSTEM_IMPLEMENTED
#include "ConsoleState.h"
#include <stdio.h>

bool GIsConsoleOpen = false;
std::vector<std::string> GLogBuffer;
std::mutex GLogMutex;

extern "C" __declspec(dllexport) void __cdecl Core_Log(const char *text) {
  if (!text)
    return;

  std::lock_guard<std::mutex> lock(GLogMutex);
  GLogBuffer.push_back(std::string(" ") + text);

  ::printf("%s\n", text);
}

extern "C" __declspec(dllexport) void __cdecl Core_ToggleConsole(bool state) {
  GIsConsoleOpen = state;
}
