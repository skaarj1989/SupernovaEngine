#include "SkeletonDebugDraw.hpp"
#include "DebugDraw.hpp"
#include "ozz/animation/runtime/skeleton_utils.h" // ozz::animation::IsLeaf
#include "animation/Conversion.hpp"

void drawSkeleton(DebugDraw &debugDraw,
                  const ozz::animation::Skeleton &skeleton,
                  ozz::span<const ozz::math::Float4x4> models,
                  const glm::mat4 &xf) {
  drawSkeleton(debugDraw, skeleton, [models, &xf](const uint32_t idx) {
    return xf * to_mat4(models[idx]);
  });
}

void drawSkeleton(DebugDraw &debugDraw,
                  const ozz::animation::Skeleton &skeleton,
                  const std::function<glm::mat4(const uint32_t)> &jointGetter) {
  constexpr glm::vec3 kBoneColor{0.596f};
  constexpr glm::vec3 kJointColor{0.396f};

  const auto &parents = skeleton.joint_parents();
  for (auto i = 0; i < skeleton.num_joints(); ++i) {
    const auto parentId = parents[i];
    auto current = jointGetter(i);
    if (parentId != ozz::animation::Skeleton::kNoParent) {
      auto parent = jointGetter(parentId);
      debugDraw.addLine(parent[3], current[3], kBoneColor);
    }
    // Only the joint is rendered for leaves, the bone model isn't.
    if (ozz::animation::IsLeaf(skeleton, i))
      debugDraw.addPoint(current[3], 6.0f, kJointColor);
  }
}
