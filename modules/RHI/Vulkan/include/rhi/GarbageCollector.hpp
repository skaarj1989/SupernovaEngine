#pragma once

#include "rhi/Buffer.hpp"
#include "rhi/Texture.hpp"
#include "rhi/FrameIndex.hpp"

namespace rhi {

class GarbageCollector {
public:
  void clear();

  void push(Buffer &);
  void push(Texture &);
  void step(const FrameIndex::ValueType threshold);

private:
  template <class T>
  using Cache = std::vector<std::pair<FrameIndex::ValueType, T>>;
  Cache<Buffer> m_buffers;
  Cache<Texture> m_textures;
};

} // namespace rhi
