#pragma once

#include "ozz/base/maths/simd_math.h"
#include "glm/ext/matrix_float4x4.hpp"

[[nodiscard]] glm::mat4 to_mat4(const ozz::math::Float4x4 &);
