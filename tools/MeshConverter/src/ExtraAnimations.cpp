#include "ExtraAnimations.hpp"
#include "assimp/SceneCombiner.h"
#include "Logger.hpp"

#include <expected>
#include <cassert>

namespace {

// Prevents animation names like "mixamo.com" or "mixamo_Hips".
constexpr auto kOverrideAnimationNames = true;

using aiAnimations = std::vector<aiAnimation *>;

[[nodiscard]] std::expected<aiAnimations, std::string>
loadExtraAnimations(Assimp::Importer &importer,
                    const std::filesystem::path &p) {
  const auto scene = importer.ReadFile(p.string(), 0);
  if (!scene) {
    return std::unexpected{importer.GetErrorString()};
  } else if (!scene->HasAnimations()) {
    return std::unexpected{"No animations."};
  }
  const auto filename = p.stem().string();

  aiAnimations animations(scene->mNumAnimations);
  for (auto i = 0u; i < animations.size(); ++i) {
    Assimp::SceneCombiner::Copy(&animations[i], scene->mAnimations[i]);
    if constexpr (kOverrideAnimationNames) {
      animations[i]->mName =
        (i == 0) ? filename : std::format("{} ({})", filename, i + 1);
    }
  }
  return animations;
}

[[nodiscard]] auto makeRawArray(std::span<aiAnimation *> in) {
  aiAnimation **out{nullptr};
  if (!in.empty()) {
    out = new aiAnimation *[in.size()];
    std::ranges::copy(in, &out[0]);
  }
  return out;
}

} // namespace

[[nodiscard]] aiAnimations
loadExtraAnimations(Assimp::Importer &importer,
                    const std::filesystem::path &dir,
                    std::span<const std::string> names) {
  aiAnimations out;
  out.reserve(names.size());
  for (const auto &n : names) {
    if (auto loadedAnimations = loadExtraAnimations(importer, dir / n);
        loadedAnimations) {
      std::ranges::copy(*loadedAnimations, std::back_inserter(out));
    } else {
      LOG_EX(warn, "[{}] {}", n, loadedAnimations.error());
    }
  }
  return out;
}

void copyAnimations(aiScene *dst, std::span<aiAnimation *> animations) {
  assert(dst);

  aiScene garbage;
  garbage.mAnimations =
    std::exchange(dst->mAnimations, makeRawArray(animations));
  garbage.mNumAnimations =
    std::exchange(dst->mNumAnimations, static_cast<int32_t>(animations.size()));
}
