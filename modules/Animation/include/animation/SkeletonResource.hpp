#pragma once

#include "Resource.hpp"
#include "ozz/animation/runtime/skeleton.h"

class SkeletonResource final : public Resource,
                               public ozz::animation::Skeleton {
public:
  SkeletonResource(ozz::animation::Skeleton &&, const std::filesystem::path &);
};
