#pragma once

#include "entt/core/type_info.hpp"
#include "RmlUi/Core/Context.h"
#include "RmlUiRenderInterface.hpp"

struct UIComponent {
  static constexpr auto in_place_delete = true;

  Rml::Context *context{nullptr};
  RmlUiRenderData renderData;

  template <typename Archive> void serialize(Archive &) {}
};

static_assert(std::is_copy_constructible_v<UIComponent>);

template <> struct entt::type_hash<UIComponent> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 1043494673;
  }
};
