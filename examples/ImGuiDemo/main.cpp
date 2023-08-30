#include "ImGuiApp.hpp"
#include "imgui.h"

class DemoApp final : public ImGuiApp {
public:
  explicit DemoApp(std::span<char *> args)
      : ImGuiApp{args, {.caption = "ImGui Demo"}} {
    ImGui::GetIO().IniFilename = nullptr;
  }

private:
  void _onPreRender() override {
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();
    ImGui::ShowDemoWindow();

    ImGui::Render();
  }
};

CONFIG_MAIN(DemoApp);
