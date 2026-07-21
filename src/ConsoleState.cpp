#define LOG_SYSTEM_IMPLEMENTED

#include "ConsoleState.h"

#include <fstream>
#include <mutex>
#include <stdio.h>

bool GIsConsoleOpen = false;
std::vector<std::string> GLogBuffer;
std::mutex GLogMutex;

extern "C" __declspec(dllexport) void __cdecl Core_Log(const char *text) {
  if (!text)
    return;

  {
    std::lock_guard<std::mutex> lock(GLogMutex);
    GLogBuffer.emplace_back(text);
  }

  ::printf("%s\n", text);

  static std::ofstream logFile("RafLoader.log", std::ios::app);

  if (logFile.is_open()) {
    logFile << text << '\n';
    logFile.flush();
  }
}

extern "C" __declspec(dllexport) void __cdecl Core_ToggleConsole(bool state) {
  GIsConsoleOpen = state;
}
