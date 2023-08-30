#pragma once

#include "Resource.hpp"
#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"

enum class ColliderType {
  Box,
  Sphere,
  Capsule,

  ConvexHull,

  TriangleMesh,
};

class ColliderResource final : public Resource {
public:
  ColliderResource(const JPH::Shape *, const std::filesystem::path &);

  const JPH::Shape *getCollisionShape() const { return m_collider.GetPtr(); }

private:
  JPH::RefConst<JPH::Shape> m_collider;
};
