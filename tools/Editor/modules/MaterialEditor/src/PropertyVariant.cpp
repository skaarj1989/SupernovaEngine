#include "PropertyVariant.hpp"
#include "VisitorHelper.hpp"

DataType getDataType(const PropertyVariant &v) {
  using enum DataType;
  return std::visit(Overload{
                      [](int32_t) { return Int32; },
                      [](uint32_t) { return UInt32; },
                      [](float) { return Float; },
                      [](glm::vec2) { return Vec2; },
                      [](const glm::vec4 &) { return Vec4; },
                    },
                    v);
}
const char *toString(const PropertyVariant &v) { return gfx::toString(v); }
