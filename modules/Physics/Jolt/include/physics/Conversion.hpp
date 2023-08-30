#pragma once

#include "glm/fwd.hpp"

class AABB;
class Transform;

namespace JPH {

class Vec3;
class Vec4;
class Quat;
class Mat44;
class AABox;

}; // namespace JPH

[[nodiscard]] JPH::Vec3 to_Jolt(const glm::vec3 &);
[[nodiscard]] JPH::Quat to_Jolt(const glm::quat &);

[[nodiscard]] glm::vec3 to_glm(const JPH::Vec3 &);
[[nodiscard]] glm::vec4 to_glm(const JPH::Vec4 &);
[[nodiscard]] glm::quat to_glm(const JPH::Quat &);
[[nodiscard]] glm::mat4 to_glm(const JPH::Mat44 &);

[[nodiscard]] AABB from_Jolt(const JPH::AABox &);