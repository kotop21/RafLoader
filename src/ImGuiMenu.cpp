#include "ImGuiMenu.h"
#include "ConsoleState.h"
#include <imgui.h>

void DrawImGuiMenu() {
  if (!GIsConsoleOpen)
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

  char windowTitle[128];
  snprintf(windowTitle, sizeof(windowTitle),
           "RafLoader Console v%s | by kotop21", RAF_VERSION);

  if (ImGui::Begin(windowTitle, &GIsConsoleOpen)) {
    std::lock_guard<std::mutex> lock(GLogMutex);

    for (const auto &log : GLogBuffer) {
      ImGui::TextUnformatted(log.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
      ImGui::SetScrollHereY(1.0f);
    }
  }
  ImGui::End();
}
