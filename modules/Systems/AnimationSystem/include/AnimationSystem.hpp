#pragma once

#include "animation/SkeletonComponent.hpp"
#include "animation/AnimationComponent.hpp"
#include "animation/PlaybackController.hpp"
#include "DebugDraw.hpp"

#include "SystemCommons.hpp"

/*
  Context variables:
  - [none]
  Components:
  - SkeletonComponent
  - AnimationComponent
  - PlaybackController
  - gfx::MeshInstance
*/
class AnimationSystem {
public:
  INTRODUCE_COMPONENTS(SkeletonComponent, AnimationComponent,
                       PlaybackController)

  static void update(entt::registry &, float dt);
  static void debugDraw(entt::registry &, DebugDraw &);
};
