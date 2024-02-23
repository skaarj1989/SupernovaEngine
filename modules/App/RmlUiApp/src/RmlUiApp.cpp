#include "RmlUiApp.hpp"
#include "VisitorHelper.hpp"
#include "RmlUi/Core/Core.h"
#ifdef _DEBUG
#  include "RmlUi/Debugger.h"
#endif
#include "RmlUiPlatformInterface.hpp"

#include "glm/ext/vector_float3.hpp"

//
// RmlUiApp class:
//

RmlUiApp::RmlUiApp(std::span<char *> args, const Config &config)
    : BaseApp{args, config} {
  m_uiFileInterface = std::make_unique<RmlUiFileInterface>();
  Rml::SetFileInterface(m_uiFileInterface.get());

  m_uiSystemInterface = std::make_unique<RmlUiSystemInterface>();
  Rml::SetSystemInterface(m_uiSystemInterface.get());

  m_uiRenderInterface =
    std::make_unique<RmlUiRenderInterface>(getRenderDevice());
  Rml::SetRenderInterface(m_uiRenderInterface.get());

  m_uiRenderData =
    m_uiRenderInterface->CreateRenderData(config.numFramesInFlight);

  Rml::Initialise();

  m_context = Rml::CreateContext("main", {0, 0});
#ifdef _DEBUG
  Rml::Debugger::Initialise(m_context);
#endif
}
RmlUiApp::~RmlUiApp() {
  getRenderDevice().waitIdle();

  Rml::Shutdown();
  m_uiRenderData = {};
  m_uiRenderInterface.reset();
  m_uiSystemInterface.reset();
  m_uiFileInterface.reset();
}

void RmlUiApp::drawGui(rhi::CommandBuffer &cb, rhi::RenderTargetView rtv,
                       std::optional<glm::vec4> clearColor) {
  auto &target = rtv.texture;
  rhi::prepareForAttachment(cb, rtv.texture, false);
  cb.beginRendering({
    .area = {.extent = target.getExtent()},
    .colorAttachments = {{
      .target = &target,
      .clearValue = std::move(clearColor),
    }},
  });
  m_uiRenderInterface->Set(cb, target, m_uiRenderData);
  m_context->Render();
  cb.endRendering();
}

void RmlUiApp::_onResizeWindow(const os::ResizeWindowEvent &evt) {
  BaseApp::_onResizeWindow(evt);
  getUiContext().SetDimensions({int32_t(evt.size.x), int32_t(evt.size.y)});
}
void RmlUiApp::_onInput(const os::InputEvent &evt) {
  BaseApp::_onInput(evt);
  processEvent(getUiContext(), evt);
}

void RmlUiApp::_onUpdate(fsec) { m_context->Update(); }

void RmlUiApp::_onRender(rhi::CommandBuffer &cb, rhi::RenderTargetView rtv,
                         fsec dt) {
  constexpr auto kClearColor = glm::vec3{0.0f};
  drawGui(cb, rtv, glm::vec4{kClearColor, 1.0f});
}
