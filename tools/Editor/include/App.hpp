#pragma once

#include "ImGuiApp.hpp"
#include "ProjectSettings.hpp"
#include "WidgetCache.hpp"
#include "renderer/CubemapConverter.hpp"
#include "renderer/WorldRenderer.hpp"
#include "sol/state.hpp"

class App final : public ImGuiApp {
public:
  explicit App(std::span<char *> args);
  ~App() override;

private:
  void _setupWidgets();

  void _onGUI();

  void _setupDockSpace(const ImGuiID) const;
  void _mainMenuBar();

  // --

  void _onInput(const os::InputEvent &) override;

  void _onUpdate(fsec dt) override;
  void _onPhysicsUpdate(fsec dt) override;

  void _onPreRender() override;
  void _onRender(rhi::CommandBuffer &, rhi::RenderTargetView, fsec dt) override;

private:
  std::optional<ProjectSettings> m_projectSettings;

  std::unique_ptr<gfx::CubemapConverter> m_cubemapConverter;
  std::unique_ptr<gfx::WorldRenderer> m_renderer;

  sol::state m_luaState;

  enum class DockSpaceSection {
    Left,
    LeftBottom,

    Center,

    Bottom,
    BottomLeft,
  };
  struct WidgetConfig {
    const char *name{nullptr};
    bool open{false};
    std::optional<DockSpaceSection> section;
  };
  WidgetCache<WidgetConfig> m_widgets;
};
