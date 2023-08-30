#include "AnimationConverter.hpp"

#include "ai2ozz.hpp"
#include "MakeSpan.hpp"
#include "Logger.hpp"

#include "ozz/animation/offline/skeleton_builder.h"
#include "ozz/animation/offline/animation_builder.h"
#include "ozz/animation/runtime/animation.h"

#include "ozz/animation/runtime/skeleton_utils.h"

#include "ozz/base/io/archive.h" // File, OArchive

#include <set>
#include <ranges>

namespace {

bool hasBones(const aiMesh *m) { return m->HasBones(); }
[[nodiscard]] auto getName(const aiBone *bone) { return bone->mName; }

[[nodiscard]] auto listBones(std::span<aiMesh *> meshes) {
  constexpr auto kComparator = [](const aiString &a, const aiString &b) {
    return std::string_view{a.C_Str()} < std::string_view{b.C_Str()};
  };
  std::set<aiString, decltype(kComparator)> out{kComparator};
  for (const auto *mesh : meshes | std::views::filter(hasBones)) {
    std::ranges::transform(MAKE_SPAN(*mesh, Bones),
                           std::inserter(out, out.begin()), getName);
  }
  return out;
}

void processBone(const aiNode *node,
                 ozz::animation::offline::RawSkeleton::Joint *joint) {
  joint->name = node->mName.C_Str();
  joint->transform = to_ozz(node->mTransformation);

  joint->children.resize(node->mNumChildren);
  for (auto i = 0u; i < joint->children.size(); ++i) {
    processBone(node->mChildren[i], &joint->children[i]);
  }
}

[[nodiscard]] std::expected<ozz::animation::offline::RawAnimation, std::string>
processAnimation(const ozz::animation::Skeleton &skeleton,
                 const aiAnimation *animation,
                 const aiMatrix4x4 &rootTransform) {
  ozz::animation::offline::RawAnimation rawAnimation;
  rawAnimation.name = animation->mName.C_Str();
  rawAnimation.duration =
    float(animation->mDuration / animation->mTicksPerSecond);
  rawAnimation.tracks.resize(skeleton.num_joints());

  for (const auto *channel : MAKE_SPAN(*animation, Channels)) {
    const auto jointIndex =
      ozz::animation::FindJoint(skeleton, channel->mNodeName.C_Str());
    assert(jointIndex != -1);

    aiVector3D scale{1.0f};
    aiQuaternion rotation;
    if (jointIndex == 0) {
      aiVector3D _;
      rootTransform.Decompose(scale, rotation, _);
    }

    const auto convert = [animation, &rotation, &scale](auto &dst, auto keys) {
      dst.reserve(keys.size());
      for (const auto &srcKey : keys) {
        auto &outKey =
          dst.emplace_back(float(srcKey.mTime / animation->mTicksPerSecond));

        using OutKeyType = std::decay_t<decltype(outKey)>;
        using ozz::animation::offline::RawAnimation;

        if constexpr (std::is_same_v<OutKeyType,
                                     RawAnimation::TranslationKey>) {
          outKey.value = to_ozz(rotation.Rotate(scale.SymMul(srcKey.mValue)));
        } else if constexpr (std::is_same_v<OutKeyType,
                                            RawAnimation::RotationKey>) {
          outKey.value = to_ozz(rotation * srcKey.mValue);
        } else if constexpr (std::is_same_v<OutKeyType,
                                            RawAnimation::ScaleKey>) {
          outKey.value = to_ozz(scale.SymMul(srcKey.mValue));
        }
      }
    };

    auto &track = rawAnimation.tracks[jointIndex];
    convert(track.translations, MAKE_SPAN(*channel, PositionKeys));
    convert(track.rotations, MAKE_SPAN(*channel, RotationKeys));
    convert(track.scales, MAKE_SPAN(*channel, ScalingKeys));
  }

  if (rawAnimation.Validate()) {
    return rawAnimation;
  } else {
    return std::unexpected{
      std::format("Corrupted animation '{}'", rawAnimation.name)};
  }
}

} // namespace

const aiNode *findRootBone(const aiScene *scene) {
  for (const auto bones = listBones(MAKE_SPAN(*scene, Meshes));
       const auto &name : bones) {
    if (const auto node = scene->mRootNode->FindNode(name);
        node && !bones.contains(node->mParent->mName)) {
      return node;
    }
  }
  return nullptr;
}

[[nodiscard]] std::expected<ozz::animation::offline::RawSkeleton, std::string>
buildRawSkeleton(const aiNode *rootBone) {
  assert(rootBone);

  ozz::animation::offline::RawSkeleton rawSkeleton;
  rawSkeleton.roots.resize(1);
  processBone(rootBone, &rawSkeleton.roots[0]);
  if (!rawSkeleton.Validate()) {
    return std::unexpected{
      std::format("Could not build the skeleton. Exceeded "
                  "the maximum number of joints ({}/{}).",
                  rawSkeleton.num_joints(),
                  std::to_underlying(ozz::animation::Skeleton::kMaxJoints))};
  }
  return rawSkeleton;
}

RuntimeSkeleton
buildRuntimeSkeleton(const ozz::animation::offline::RawSkeleton &rawSkeleton) {
  ozz::animation::offline::SkeletonBuilder builder;
  return builder(rawSkeleton);
}

bool exportSkeleton(const ozz::animation::Skeleton &skeleton,
                    const std::filesystem::path &dir) {
  ozz::io::File f{(dir / "skeleton.ozz").string().c_str(), "wb"};
  if (!f.opened()) return false;

  ozz::io::OArchive archive{&f};
  archive << skeleton;
  return true;
}

AnimationList processAnimations(const ozz::animation::Skeleton &skeleton,
                                std::span<aiAnimation *> animations,
                                const aiMatrix4x4 &rootTransform) {
  AnimationList out;
  for (const auto *animation : animations) {
    if (auto rawAnimation =
          processAnimation(skeleton, animation, rootTransform);
        rawAnimation) {
      out.emplace_back(std::move(*rawAnimation));
    } else {
      LOG(error, rawAnimation.error());
    }
  }
  return out;
}
std::size_t exportAnimations(const AnimationList &rawAnimations,
                             const std::filesystem::path &dir) {
  std::filesystem::create_directories(dir);

  std::size_t numExported{0};
  ozz::animation::offline::AnimationBuilder builder;
  for (const auto &rawAnimation : rawAnimations) {
    const auto animation = builder(rawAnimation);
    const auto p = (dir / animation->name()).replace_extension("ozz");
    if (ozz::io::File f{p.string().c_str(), "wb"}; f.opened()) {
      ozz::io::OArchive archive{&f};
      archive << *animation;
      ++numExported;
    }
  }
  return numExported;
}
