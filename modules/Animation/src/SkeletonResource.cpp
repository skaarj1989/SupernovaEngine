#include "animation/SkeletonResource.hpp"

SkeletonResource::SkeletonResource(ozz::animation::Skeleton &&skeleton,
                                   const std::filesystem::path &p)
    : Resource{p}, ozz::animation::Skeleton{std::move(skeleton)} {}
