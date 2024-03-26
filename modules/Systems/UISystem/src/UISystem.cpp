#include "UISystem.hpp"
#include "rhi/RenderDevice.hpp"
#include "RmlUi/Core/Core.h"
#include "RmlUi/Core/Context.h"
#include "CameraComponent.hpp"

#include <format>

namespace {

[[nodiscard]] Rml::Vector2i to_Rml(const rhi::Extent2D extent) {
  return {int32_t(extent.width), int32_t(extent.height)};
}

auto &getRenderDevice(entt::registry &r) {
  return r.ctx().get<RmlUiRenderInterface *>()->GetRenderer().getRenderDevice();
}

void initUIComponent(entt::registry &r, const entt::entity e) {
  const auto *renderInterface = r.ctx().get<RmlUiRenderInterface *>();
  auto &ui = r.get<UIComponent>(e);
  ui.renderData = renderInterface->CreateRenderData(2);
  const auto name =
    std::format("{}_{}", std::bit_cast<intptr_t>(&r), entt::to_integral(e));

  const auto *cc = r.try_get<CameraComponent>(e);
  ui.context =
    Rml::CreateContext(name, cc ? to_Rml(cc->extent) : Rml::Vector2i{0, 0});
}
void cleanupUIComponent(entt::registry &r, const entt::entity e) {
  if (auto &ui = r.get<UIComponent>(e); ui.context) {
    auto &rd = getRenderDevice(r);
    for (auto &resources : ui.renderData.frameResources) {
      rd.pushGarbage(resources.vertexBuffer).pushGarbage(resources.indexBuffer);
    }
    Rml::RemoveContext(ui.context->GetName());
    ui.context = nullptr;
  }
}
void connectUIComponent(entt::registry &r) {
  r.on_construct<UIComponent>().connect<&initUIComponent>();
  r.on_destroy<UIComponent>().connect<&cleanupUIComponent>();
}

} // namespace

//
// UISystem class:
//

void UISystem::setup(entt::registry &r, RmlUiRenderInterface &renderInterface) {
  r.ctx().emplace<RmlUiRenderInterface *>(&renderInterface);
  connectUIComponent(r);
}

void UISystem::update(entt::registry &r) {
  ZoneScopedN("UISystem::Update");
  for (auto [_, ui, cc] : r.view<UIComponent, CameraComponent>().each()) {
    if (ui.context) ui.context->Update();
  }
}
void UISystem::render(entt::registry &r, rhi::CommandBuffer &cb) {
  ZoneScopedN("UISystem::Render");

  auto *renderInterface = r.ctx().get<RmlUiRenderInterface *>();
  assert(renderInterface);

  for (auto [_, ui, cc] : r.view<UIComponent, CameraComponent>().each()) {
    if (!ui.context && (!cc.target || !(*cc.target))) continue;

    RHI_GPU_ZONE(cb, "RmlUi");
    rhi::prepareForAttachment(cb, *cc.target, false);
    cb.beginRendering({
      .area = {.extent = cc.target->getExtent()},
      .colorAttachments = {{.target = cc.target.get()}},
    });
    renderInterface->Set(cb, *cc.target, ui.renderData);
    ui.context->Render();
    cb.endRendering();
  }
}
