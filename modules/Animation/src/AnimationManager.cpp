#include "animation/AnimationManager.hpp"
#include "LoaderHelper.hpp"

AnimationResourceHandle AnimationManager::load(const std::filesystem::path &p) {
  return ::load(*this, p, LoadMode::External);
}
