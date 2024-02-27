#pragma once

#ifdef RHI_USE_DEBUG_MARKER
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

#  define RHI_DEBUG_MARKER_ID(Name, ID) _RHI_DEBUG_MARKER_ID(Name, ID)
#  define _RHI_DEBUG_MARKER_ID(Name, ID) Name##ID

#  define RHI_NAMED_DEBUG_MARKER(CommandBuffer, Label)                         \
    const rhi::DebugMarker RHI_DEBUG_MARKER_ID(_debug_marker, __LINE__) {      \
      CommandBuffer, Label                                                     \
    }
#  define RHI_DEBUG_MARKER(CommandBuffer)                                      \
    RHI_NAMED_DEBUG_MARKER(CommandBuffer, __FUNCTION__)

} // namespace rhi
#else
#  define RHI_NAMED_DEBUG_MARKER(CommandBuffer, Label)
#  define RHI_DEBUG_MARKER(CommandBuffer)
#endif
