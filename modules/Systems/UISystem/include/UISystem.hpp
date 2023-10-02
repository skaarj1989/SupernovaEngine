#pragma once

#include "entt/entity/registry.hpp"
#include "UIComponent.hpp"
#include "SystemCommons.hpp"

/*
  Context variables:
  - RmlUiRenderInterface *
  Components:
  - UIComponent
  - CameraComponent
*/
class UISystem {
public:
  INTRODUCE_COMPONENTS(UIComponent)

  static void setup(entt::registry &, RmlUiRenderInterface &);

  static void update(entt::registry &);
  static void render(entt::registry &, rhi::CommandBuffer &);
};
