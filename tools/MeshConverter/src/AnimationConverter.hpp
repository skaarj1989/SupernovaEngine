#pragma once

#include "assimp/scene.h"

#include "ozz/base/memory/unique_ptr.h"
#include "ozz/animation/offline/raw_skeleton.h"
#include "ozz/animation/offline/raw_animation.h"
#include "ozz/animation/runtime/skeleton.h"

#include <expected>
#include <span>
#include <filesystem>

[[nodiscard]] const aiNode *findRootBone(const aiScene *);

[[nodiscard]] std::expected<ozz::animation::offline::RawSkeleton, std::string>
buildRawSkeleton(const aiNode *rootBone);

using RuntimeSkeleton = ozz::unique_ptr<ozz::animation::Skeleton>;
[[nodiscard]] RuntimeSkeleton
buildRuntimeSkeleton(const ozz::animation::offline::RawSkeleton &);

bool exportSkeleton(const ozz::animation::Skeleton &,
                    const std::filesystem::path &dir);

using AnimationList = std::vector<ozz::animation::offline::RawAnimation>;

[[nodiscard]] AnimationList
processAnimations(const ozz::animation::Skeleton &skeleton,
                  std::span<aiAnimation *>, const aiMatrix4x4 &rootTransform);
std::size_t exportAnimations(const AnimationList &,
                             const std::filesystem::path &dir);
