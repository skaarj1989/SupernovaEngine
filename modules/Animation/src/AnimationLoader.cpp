#include "animation/AnimationLoader.hpp"
#include "TryLoad.hpp"
#include "spdlog/spdlog.h"

AnimationLoader::result_type
AnimationLoader::operator()(const std::filesystem::path &p) const {
  if (auto animation = tryLoad<ozz::animation::Animation>(p); animation) {
    return std::make_shared<AnimationResource>(std::move(animation.value()),
                                               p.lexically_normal());
  } else {
    SPDLOG_ERROR("Animation loading failed. {}", animation.error());
    return {};
  }
}
