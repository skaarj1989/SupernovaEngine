#pragma once

#include "renderer/FrameGraphBuffer.hpp"

namespace gfx {

template <typename T> struct TransientBuffer {
  const std::string_view name;
  BufferType type;
  T data;
};

} // namespace gfx
