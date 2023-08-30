#pragma once

#include "ozz/base/maths/simd_math.h"
#include "glm/fwd.hpp"

[[nodiscard]] glm::mat4 to_mat4(const ozz::math::Float4x4 &);
