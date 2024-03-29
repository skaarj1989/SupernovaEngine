#pragma once

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"

#include <string>
#include <variant>

namespace gfx {

struct Property {
  std::string name;
  using Value = std::variant<int32_t, uint32_t, float, glm::vec2, glm::vec4>;
  Value value;

  template <class Archive> void serialize(Archive &archive) {
    archive(name, value);
  }
};

// @return sizeof internal type
[[nodiscard]] uint32_t getSize(const Property::Value &);
[[nodiscard]] const char *toString(const Property::Value &);

} // namespace gfx
