#pragma once

#include "renderer/FrameGraphBuffer.hpp"

namespace gfx {

#pragma warning(push)
// structure was padded due to alignment specifier
#pragma warning(disable : 4324)

template <typename T> struct TransientBuffer {
  const std::string_view name;
  BufferType type;
  T data;
};

#pragma warning(pop)

} // namespace gfx
