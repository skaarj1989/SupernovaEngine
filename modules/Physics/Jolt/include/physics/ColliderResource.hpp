#pragma once

#include "Resource.hpp"

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

class ColliderResource final : public Resource {
public:
  ColliderResource(const JPH::Shape *, const std::filesystem::path &);

  const JPH::Shape *getCollisionShape() const { return m_collider.GetPtr(); }

private:
  JPH::RefConst<JPH::Shape> m_collider;
};
