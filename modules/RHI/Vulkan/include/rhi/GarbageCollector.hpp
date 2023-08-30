#pragma once

#include "rhi/Buffer.hpp"
#include "rhi/Texture.hpp"

namespace rhi {

class GarbageCollector {
public:
  void clear();

  void push(Buffer &);
  void push(Texture &);
  void step();

private:
  template <class T> using Cache = std::vector<std::pair<std::size_t, T>>;
  Cache<Buffer> m_buffers;
  Cache<Texture> m_textures;
};

} // namespace rhi
