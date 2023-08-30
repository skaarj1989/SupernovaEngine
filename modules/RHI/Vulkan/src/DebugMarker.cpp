#ifdef _DEBUG
#  include "rhi/DebugMarker.hpp"
#  include "rhi/CommandBuffer.hpp"

namespace rhi {

DebugMarker::DebugMarker(CommandBuffer &cb, const std::string_view label)
    : m_commandBuffer{cb} {
  m_commandBuffer._pushDebugGroup(label);
}
DebugMarker::~DebugMarker() { m_commandBuffer._popDebugGroup(); }

} // namespace rhi
#endif
