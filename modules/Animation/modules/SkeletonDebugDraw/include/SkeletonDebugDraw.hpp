#pragma once

#include "ozz/base/span.h"
#include "ozz/base/maths/simd_math.h"
#include "glm/fwd.hpp"
#include <functional>

namespace ozz::animation {
class Skeleton;
}

class DebugDraw;

void drawSkeleton(DebugDraw &, const ozz::animation::Skeleton &,
                  ozz::span<const ozz::math::Float4x4> models,
                  const glm::mat4 &);

void drawSkeleton(DebugDraw &, const ozz::animation::Skeleton &,
                  const std::function<glm::mat4(const uint32_t)> &jointGetter);
