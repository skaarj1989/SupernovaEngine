#include "rhi/GarbageCollector.hpp"
#include <algorithm>

#define SAFE_DELETE(p)                                                         \
  if (p) {                                                                     \
    delete p;                                                                  \
    p = nullptr;                                                               \
  }

namespace rhi {

void GarbageCollector::clear() {
  static constexpr auto deleteResource = [](auto &p) { SAFE_DELETE(p.second); };

  std::ranges::for_each(m_textures, deleteResource);
  m_textures.clear();

  std::ranges::for_each(m_buffers, deleteResource);
  m_buffers.clear();
}

void GarbageCollector::step(const FrameIndex::ValueType threshold) {
  static constexpr auto inc = [](auto &p) { ++p.first; };
  const auto expired = [threshold](auto &p) {
    if (p.first <= threshold) return false;

    SAFE_DELETE(p.second);
    return true;
  };
  const auto step = [expired](auto &cache) {
    if (cache.empty()) return;

    std::ranges::for_each(cache, inc);
    const auto [first, last] = std::ranges::remove_if(cache, expired);
    cache.erase(first, last);
  };
  step(m_buffers);
  step(m_textures);
}

} // namespace rhi
