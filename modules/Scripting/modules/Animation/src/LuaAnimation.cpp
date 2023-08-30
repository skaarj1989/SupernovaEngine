#include "LuaAnimation.hpp"
#include "sol/sol.hpp"

#include "animation/SkeletonComponent.hpp"
#include "animation/AnimationComponent.hpp"
#include "animation/PlaybackController.hpp"

#include "Sol2HelperMacros.hpp"

namespace {

void registerResources(sol::state &lua) {
  // clang-format off
  DEFINE_USERTYPE(SkeletonResource,
    sol::no_constructor,
    sol::base_classes, sol::bases<Resource, ozz::animation::Skeleton>(),

    BIND_TOSTRING(SkeletonResource)
  );
  lua["loadSkeleton"] = loadResource<SkeletonManager>;

  DEFINE_USERTYPE(AnimationResource,
    sol::no_constructor,
    sol::base_classes, sol::bases<Resource, ozz::animation::Animation>(),

    BIND_TOSTRING(AnimationResource)
  );
  lua["loadAnimation"] = loadResource<AnimationManager>;
  // clang-format on
}

void registerSkeletonComponent(sol::state &lua) {
  // clang-format off
  DEFINE_USERTYPE(SkeletonComponent,
    sol::call_constructor,
    sol::factories(
      [](std::shared_ptr<SkeletonResource> resource) {
        return SkeletonComponent{std::move(resource)};
      }
    ),

    "resource", sol::property(
      [](const SkeletonComponent &self) { return self.resource; },
      [](SkeletonComponent &self, std::shared_ptr<SkeletonResource> resource) {
        self.resource = std::move(resource);
      }
    ),

    BIND_TYPEID(SkeletonComponent),
    BIND_TOSTRING(SkeletonComponent)
  );
  // clang-format on
}
void registerAnimationComponent(sol::state &lua) {
  // clang-format off
  DEFINE_USERTYPE(AnimationComponent,
    sol::call_constructor,
    sol::factories(
      [](std::shared_ptr<AnimationResource> resource) {
        return AnimationComponent{std::move(resource)};
      }
    ),

    "resource", sol::property(
      [](const AnimationComponent &self) { return self.resource; },
      [](AnimationComponent &self, std::shared_ptr<AnimationResource> resource) {
        self.resource = std::move(resource);
      }
    ),

    BIND_TYPEID(AnimationComponent),
    BIND_TOSTRING(AnimationComponent)
  );
  // clang-format on
}
void registerPlaybackController(sol::state &lua) {
#define BIND(Member) _BIND(PlaybackController, Member)
  // clang-format off
  DEFINE_USERTYPE(PlaybackController,
    sol::call_constructor,
    sol::constructors<PlaybackController()>(),

    BIND(setTimeRatio),
    BIND(setSpeed),
    BIND(setLoop),

    BIND(getTimeRatio),
    BIND(getPreviousTimeRatio),
    BIND(getPlaybackSpeed),

    BIND(isPlaying),
    BIND(isLooped),
    
    BIND(play),
    BIND(pause),
    BIND(resume),
    BIND(stop),

    BIND(reset),

    BIND_TYPEID(PlaybackController),
    BIND_TOSTRING(PlaybackController)
  );
  // clang-format on
#undef BIND
}

} // namespace

void registerAnimation(sol::state &lua) {
  registerResources(lua);
  registerSkeletonComponent(lua);
  registerAnimationComponent(lua);
  registerPlaybackController(lua);
}
