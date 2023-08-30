#include "animation/AnimationResource.hpp"

AnimationResource::AnimationResource(ozz::animation::Animation &&animation,
                                     const std::filesystem::path &p)
    : Resource{p}, ozz::animation::Animation{std::move(animation)} {}
