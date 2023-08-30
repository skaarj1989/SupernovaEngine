#include "rhi/FrameIndex.hpp"
#include <utility> // exchange
#include <cassert>

namespace rhi {

FrameIndex::FrameIndex(uint32_t framesInFlight)
    : m_framesInFlight{framesInFlight} {}

void FrameIndex::operator++() {
  assert(m_framesInFlight != 0);
  m_previous = std::exchange(m_index, (m_index + 1) % m_framesInFlight);
}
FrameIndex::operator int32_t() const { return m_index; }

uint32_t FrameIndex::getCurrentIndex() const { return m_index; }
uint32_t FrameIndex::getPreviousIndex() const { return m_previous; }

} // namespace rhi
