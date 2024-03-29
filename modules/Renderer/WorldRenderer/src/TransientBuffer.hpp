#pragma once

#include "renderer/FrameGraphBuffer.hpp"

namespace gfx {

#if defined(_MSC_VER)
#  pragma warning(push)
// structure was padded due to alignment specifier
#  pragma warning(disable : 4324)
#endif

template <typename T> struct TransientBuffer {
  const std::string_view name;
  BufferType type;
  T data;
};

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif

} // namespace gfx
