#include "renderer/MaterialProperty.hpp"
#include "VisitorHelper.hpp"

namespace gfx {

uint32_t getSize(const Property::Value &v) {
  return std::visit(
    [](const auto &in) { return static_cast<uint32_t>(sizeof(in)); }, v);
}
const char *toString(const Property::Value &v) {
  return std::visit(Overload{
                      [](const int32_t) { return "int"; },
                      [](const uint32_t) { return "uint"; },
                      [](const float) { return "float"; },
                      [](const glm::vec2) { return "vec2"; },
                      [](const glm::vec4) { return "vec4"; },
                    },
                    v);
}

} // namespace gfx
