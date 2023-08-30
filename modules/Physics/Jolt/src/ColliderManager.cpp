#include "physics/ColliderManager.hpp"
#include "LoaderHelper.hpp"

ColliderResourceHandle ColliderManager::load(const std::filesystem::path &p) {
  return ::load(*this, p, LoadMode::External);
}
