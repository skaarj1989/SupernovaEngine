#pragma once

#include <cstdint>

namespace rhi {

struct FrameIndex {
  explicit FrameIndex(uint32_t framesInFlight = 0);

  void operator++();
  // Implicit conversion operator for convenience.
  [[nodiscard]] operator int32_t() const;

  [[nodiscard]] uint32_t getCurrentIndex() const;
  [[nodiscard]] uint32_t getPreviousIndex() const;

private:
  uint32_t m_framesInFlight{0};
  uint32_t m_index{0};
  uint32_t m_previous{0};
};

} // namespace rhi
