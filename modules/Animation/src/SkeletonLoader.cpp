#include "animation/SkeletonLoader.hpp"
#include "TryLoad.hpp"
#include "spdlog/spdlog.h"

std::shared_ptr<SkeletonResource>
SkeletonLoader::operator()(const std::filesystem::path &p) const {
  if (auto skeleton = tryLoad<ozz::animation::Skeleton>(p); skeleton) {
    return std::make_shared<SkeletonResource>(std::move(skeleton.value()),
                                              p.lexically_normal());
  } else {
    SPDLOG_ERROR("Skeleton loading failed. {}", skeleton.error());
    return {};
  }
}
