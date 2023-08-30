#pragma once

#include "MaterialEditor/Nodes/NodeCommon.hpp"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat2x2.hpp"
#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"

#include <variant>

// clang-format off

// Basic types (non-opaque, scalar/vector/matrix type).
using ValueVariant = std::variant<
  bool, glm::bvec2, glm::bvec3, glm::bvec4,
  int32_t, glm::ivec2, glm::ivec3, glm::ivec4,
  uint32_t, glm::uvec2, glm::uvec3, glm::uvec4,
  float, glm::vec2, glm::vec3, glm::vec4,
  double, glm::dvec2, glm::dvec3, glm::dvec4,
  glm::mat2, glm::mat3, glm::mat4
>;
// clang-format on

[[nodiscard]] DataType getDataType(const ValueVariant &);

[[nodiscard]] const char *toString(const ValueVariant &);

bool inspectNode(int32_t id, std::optional<const char *> userLabel,
                 ValueVariant &);
bool inspect(ValueVariant &);
bool changeValueCombo(const char *label, ValueVariant &);

[[nodiscard]] NodeResult evaluate(MaterialGenerationContext &, int32_t id,
                                  const ValueVariant &);
