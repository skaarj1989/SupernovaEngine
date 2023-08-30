#pragma once

#include "SystemSerializationContext.hpp"

#define INTRODUCE_COMPONENTS(...)                                              \
  static constexpr entt::type_list<__VA_ARGS__> kIntroducedComponents{};
