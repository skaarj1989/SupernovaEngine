#include "rhi/GarbageCollector.hpp"
#include <algorithm>

namespace rhi {

void GarbageCollector::clear() {
  m_textures.clear();
  m_buffers.clear();
}

void GarbageCollector::push(Buffer &buffer) {
  m_buffers.emplace_back(0, std::move(buffer));
}
void GarbageCollector::push(Texture &texture) {
  m_textures.emplace_back(0, std::move(texture));
}

void GarbageCollector::step() {
  constexpr auto step = [](auto &cache) {
    constexpr auto inc = [](auto &p) { ++p.first; };
    constexpr auto expired = [](auto &p) {
      constexpr auto kThreshold = 2;
      return p.first >= kThreshold;
    };
    if (cache.empty()) return;

    std::ranges::for_each(cache, inc);
    const auto [first, last] = std::ranges::remove_if(cache, expired);
    cache.erase(first, last);
  };
  step(m_buffers);
  step(m_textures);
}

} // namespace rhi
