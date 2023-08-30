#pragma once

#ifdef _DEBUG
#  include <string_view>

namespace rhi {

class CommandBuffer;

class DebugMarker final {
public:
  DebugMarker() = delete;
  DebugMarker(CommandBuffer &, const std::string_view label);
  DebugMarker(const DebugMarker &) = delete;
  DebugMarker(DebugMarker &&) = delete;
  ~DebugMarker();

  DebugMarker &operator=(const DebugMarker &) = delete;
  DebugMarker &operator=(DebugMarker &&) noexcept = delete;

private:
  CommandBuffer &m_commandBuffer;
};

#  define DEBUG_MARKER_ID(name, id) _DEBUG_MARKER_ID(name, id)
#  define _DEBUG_MARKER_ID(name, id) name##id

#  define NAMED_DEBUG_MARKER(cb, label)                                        \
    const rhi::DebugMarker DEBUG_MARKER_ID(dm, __LINE__) { cb, label }
#  define DEBUG_MARKER(cb) NAMED_DEBUG_MARKER(cb, __FUNCTION__)

} // namespace rhi
#else
#  define NAMED_DEBUG_MARKER(cb, name)
#  define DEBUG_MARKER(cb)
#endif
