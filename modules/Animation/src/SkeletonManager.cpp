#include "animation/SkeletonManager.hpp"
#include "LoaderHelper.hpp"

SkeletonResourceHandle SkeletonManager::load(const std::filesystem::path &p) {
  return ::load(*this, p, LoadMode::External);
}
