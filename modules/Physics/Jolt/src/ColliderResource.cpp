#include "physics/ColliderResource.hpp"

ColliderResource::ColliderResource(const JPH::Shape *shape,
                                   const std::filesystem::path &p)
    : Resource{p}, m_collider{shape} {
  assert(shape);
}
