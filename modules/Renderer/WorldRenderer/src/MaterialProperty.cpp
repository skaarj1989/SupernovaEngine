#include "renderer/MaterialProperty.hpp"
#include "VisitorHelper.hpp"

namespace gfx {

std::size_t getSize(const Property::Value &v) {
  return std::visit(Overload{
                      [](int32_t) { return sizeof(int32_t); },
                      [](uint32_t) { return sizeof(uint32_t); },
                      [](float) { return sizeof(float); },
                      [](glm::vec2) { return sizeof(glm::vec2); },
                      [](glm::vec4) { return sizeof(glm::vec4); },
                    },
                    v);
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
