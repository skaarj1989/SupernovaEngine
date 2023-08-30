#pragma once

#include "SystemCommons.hpp"
#include "DebugDraw.hpp"
#include "animation/SkeletonComponent.hpp"
#include "animation/AnimationComponent.hpp"
#include "animation/PlaybackController.hpp"

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

  template <class Archive> static void save(Archive &archive) {
    auto &[_, snapshot] = cereal::get_user_data<OutputContext>(archive);
    snapshot.get<SkeletonComponent>(archive)
      .get<AnimationComponent>(archive)
      .get<PlaybackController>(archive);
  }
  template <class Archive> static void load(Archive &archive) {
    auto &[_, snapshotLoader] = cereal::get_user_data<InputContext>(archive);
    snapshotLoader.get<SkeletonComponent>(archive)
      .get<AnimationComponent>(archive)
      .get<PlaybackController>(archive);
  }
};
