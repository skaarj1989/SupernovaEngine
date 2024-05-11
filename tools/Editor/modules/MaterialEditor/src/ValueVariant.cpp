#include "ValueVariant.hpp"
#include "VisitorHelper.hpp"

DataType getDataType(const ValueVariant &v) {
  using enum DataType;
  return std::visit(Overload{
                      [](bool) { return Bool; },
                      [](glm::bvec2) { return BVec2; },
                      [](glm::bvec3) { return BVec3; },
                      [](glm::bvec4) { return BVec4; },

                      [](int32_t) { return Int32; },
                      [](glm::ivec2) { return IVec2; },
                      [](glm::ivec3) { return IVec3; },
                      [](glm::ivec4) { return IVec4; },

                      [](uint32_t) { return UInt32; },
                      [](glm::uvec2) { return UVec2; },
                      [](glm::uvec3) { return UVec3; },
                      [](glm::uvec4) { return UVec4; },

                      [](float) { return Float; },
                      [](glm::vec2) { return Vec2; },
                      [](glm::vec3) { return Vec3; },
                      [](glm::vec4) { return Vec4; },

                      [](double) { return Double; },
                      [](glm::dvec2) { return DVec2; },
                      [](glm::dvec3) { return DVec3; },
                      [](const glm::dvec4 &) { return DVec4; },

                      [](const glm::mat2 &) { return Mat2; },
                      [](const glm::mat3 &) { return Mat3; },
                      [](const glm::mat4 &) { return Mat4; },
                    },
                    v);
}
const char *toString(const ValueVariant &v) { return toString(getDataType(v)); }
