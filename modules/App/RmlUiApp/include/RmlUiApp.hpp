#pragma once

#include "BaseApp.hpp"
#include "RmlUiPlatformInterface.hpp"
#include "RmlUiRenderInterface.hpp"
#include "RmlUi/Core/Context.h"

class RmlUiApp : public BaseApp {
public:
  RmlUiApp(std::span<char *> args, const Config &);
  ~RmlUiApp() override;

  void drawGui(rhi::CommandBuffer &, rhi::RenderTargetView,
               std::optional<glm::vec4> clearColor);

  Rml::Context &getUiContext() { return *m_context; }

protected:
  void _onResizeWindow(const os::ResizeWindowEvent &) override;
  void _onInput(const os::InputEvent &) override;

  void _onUpdate(fsec) override;

  void _onRender(rhi::CommandBuffer &, rhi::RenderTargetView, fsec dt) override;

private:
  Rml::Context *m_context{nullptr};

  std::unique_ptr<RmlUiFileInterface> m_uiFileInterface;
  std::unique_ptr<RmlUiSystemInterface> m_uiSystemInterface;
  std::unique_ptr<RmlUiRenderInterface> m_uiRenderInterface;
  RmlUiRenderData m_uiRenderData;
};
