#pragma once

#include "assimp/vector3.h"
#include "assimp/color4.h"
#include "assimp/matrix4x4.h"
#include "assimp/quaternion.h"
#include "assimp/aabb.h"

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/quaternion_float.hpp"

#include "math/AABB.hpp"

[[nodiscard]] glm::vec2 to_vec2(const aiVector3D &);
[[nodiscard]] glm::vec3 to_vec3(const aiVector3D &);
[[nodiscard]] glm::vec4 to_vec4(const aiColor4D &);
[[nodiscard]] glm::mat4 to_mat4(const aiMatrix4x4 &);
[[nodiscard]] glm::quat to_quat(const aiQuaternion &);

[[nodiscard]] AABB to_aabb(const aiAABB &);
