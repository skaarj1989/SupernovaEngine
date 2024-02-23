#include "rhi/FrameIndex.hpp"
#include <utility> // exchange
#include <cassert>

namespace rhi {

FrameIndex::FrameIndex(const FrameIndex::ValueType numFramesInFlight)
    : m_numFramesInFlight{numFramesInFlight} {}

void FrameIndex::operator++() {
  assert(m_numFramesInFlight != 0);
  m_previous = std::exchange(m_index, (m_index + 1) % m_numFramesInFlight);
}
FrameIndex::operator ValueType() const { return m_index; }

FrameIndex::ValueType FrameIndex::getCurrentIndex() const { return m_index; }
FrameIndex::ValueType FrameIndex::getPreviousIndex() const {
  return m_previous;
}

} // namespace rhi
