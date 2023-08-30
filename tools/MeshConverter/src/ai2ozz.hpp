#pragma once

#include "assimp/matrix4x4.h"
#include "assimp/quaternion.h"
#include "ozz/base/maths/transform.h"

[[nodiscard]] ozz::math::Float3 to_ozz(const aiVector3D &);
[[nodiscard]] ozz::math::Transform to_ozz(const aiMatrix4x4 &);
[[nodiscard]] ozz::math::Quaternion to_ozz(const aiQuaternion &);
