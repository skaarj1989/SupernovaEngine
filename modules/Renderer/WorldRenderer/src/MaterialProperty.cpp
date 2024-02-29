#include "renderer/MaterialProperty.hpp"
#include "VisitorHelper.hpp"

namespace gfx {

std::size_t getSize(const Property::Value &v) {
  return std::visit([](const auto &in) { return sizeof(in); }, v);
}
const char *toString(const Property::Value &v) {
  return std::visit(Overload{
                      [](int32_t) { return "int"; },
                      [](uint32_t) { return "uint"; },
                      [](float) { return "float"; },
                      [](glm::vec2) { return "vec2"; },
                      [](glm::vec4) { return "vec4"; },
                    },
                    v);
}

} // namespace gfx
