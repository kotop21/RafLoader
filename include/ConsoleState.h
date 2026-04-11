#pragma once
#include <mutex>
#include <stdio.h>
#include <string>
#include <vector>

extern bool GIsConsoleOpen;
extern std::vector<std::string> GLogBuffer;
extern std::mutex GLogMutex;

extern "C" __declspec(dllexport) void __cdecl Core_Log(const char *text);
extern "C" __declspec(dllexport) void __cdecl Core_ToggleConsole(bool state);

#ifndef LOG_SYSTEM_IMPLEMENTED
#define printf(fmt, ...)                                                       \
  {                                                                            \
    char _tmp_buf[4096];                                                       \
    snprintf(_tmp_buf, sizeof(_tmp_buf), fmt, ##__VA_ARGS__);                  \
    Core_Log(_tmp_buf);                                                        \
  }
#endif
