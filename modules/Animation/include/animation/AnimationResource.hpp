#pragma once

#include "Resource.hpp"
#include "ozz/animation/runtime/animation.h"

class AnimationResource final : public Resource,
                                public ozz::animation::Animation {
public:
  AnimationResource(ozz::animation::Animation &&,
                    const std::filesystem::path &);
};
