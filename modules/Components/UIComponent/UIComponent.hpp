#pragma once

#include "RmlUi/Core/Context.h"
#include "RmlUiRenderInterface.hpp"

struct UIComponent {
  static constexpr auto in_place_delete = true;

  Rml::Context *context{nullptr};
  RmlUiRenderData renderData;

  template <typename Archive> void serialize(Archive &) {}
};

static_assert(std::is_copy_constructible_v<UIComponent>);
