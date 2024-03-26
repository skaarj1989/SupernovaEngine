#include "AnimationSystem.hpp"
#include "renderer/MeshInstance.hpp"

#include "ozz/base/maths/soa_transform.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/animation/runtime/local_to_model_job.h"

#include "animation/Conversion.hpp"
#include "SkeletonDebugDraw.hpp"

#include "tracy/Tracy.hpp"

namespace {

[[nodiscard]] auto
buildSkin(std::span<const gfx::Joints::value_type> inverseBindPose,
          ozz::span<const ozz::math::Float4x4> models) {
  ZoneScopedN("BuildSkin");
  gfx::Joints skin;
  if (inverseBindPose.size() == models.size()) {
    skin.reserve(inverseBindPose.size());
    for (auto i = 0u; i < models.size(); ++i) {
      skin.emplace_back(to_mat4(models[i]) * inverseBindPose[i]);
    }
  }
  return skin;
}
[[nodiscard]] auto buildSkin(const gfx::Mesh &mesh,
                             ozz::span<const ozz::math::Float4x4> models) {
  return buildSkin(mesh.getInverseBindPose(), models);
}

void drawSkeleton(DebugDraw &debugDraw,
                  const ozz::animation::Skeleton &skeleton,
                  const gfx::MeshInstance &meshInstance) {
  if (!meshInstance.hasSkin()) return;

  drawSkeleton(debugDraw, skeleton, [&meshInstance](const uint32_t idx) {
    const auto &skin = meshInstance.getSkinMatrices();
    const auto &inversedBindPose = meshInstance->getInverseBindPose();
    if (skin.size() != inversedBindPose.size()) return glm::mat4{1.0f};

    return meshInstance.getModelMatrix() * skin[idx] *
           glm::inverse(inversedBindPose[idx]);
  });
}

} // namespace

void AnimationSystem::update(entt::registry &r, const float dt) {
  ZoneScopedN("AnimationSystem::Update");
  for (auto [_, meshInstance, skeleton, animation, controller] :
       r.view<gfx::MeshInstance, const SkeletonComponent,
              const AnimationComponent, PlaybackController>()
         .each()) {
    const auto *prototype = meshInstance.getPrototype().get();
    if (!prototype || !prototype->isSkeletal() || !skeleton.resource ||
        !animation.resource) {
      continue;
    }

    controller.update(*animation.resource, dt);

    std::vector<ozz::math::SoaTransform> locals(
      skeleton.resource->num_soa_joints());

    ozz::animation::SamplingJob::Context context;
    context.Resize(skeleton.resource->num_joints());
    ozz::animation::SamplingJob samplingJob;
    {
      ZoneScopedN("SamplingJob");
      samplingJob.animation = animation.resource.get();
      samplingJob.context = &context;
      samplingJob.ratio = controller.getTimeRatio();
      samplingJob.output = ozz::make_span(locals);
      if (!samplingJob.Run()) {
        continue;
      }
    }

    std::vector<ozz::math::Float4x4> models(skeleton.resource->num_joints());
    ozz::animation::LocalToModelJob localToModelJob;
    {
      ZoneScopedN("LocalToModelJob");
      localToModelJob.skeleton = skeleton.resource.get();
      localToModelJob.input = ozz::make_span(locals);
      localToModelJob.output = ozz::make_span(models);
      if (!localToModelJob.Run()) {
        continue;
      }
    }

    meshInstance.setSkinMatrices(buildSkin(*prototype, localToModelJob.output));
  }
}

void AnimationSystem::debugDraw(entt::registry &r, DebugDraw &dd) {
  ZoneScopedN("AnimationSystem::DebugDraw");
  for (auto [_, meshInstance, skeleton, animation] :
       r.view<const gfx::MeshInstance, const SkeletonComponent,
              const AnimationComponent>()
         .each()) {
    const auto *prototype = meshInstance.getPrototype().get();
    if (prototype && prototype->isSkeletal() && skeleton.resource &&
        animation.resource) {
      drawSkeleton(dd, *skeleton.resource, meshInstance);
    }
  }
}
