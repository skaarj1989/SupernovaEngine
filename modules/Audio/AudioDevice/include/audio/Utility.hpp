#pragma once

#include "glm/common.hpp" // clamp

namespace audio {

[[nodiscard]] inline float dBToVolume(const int8_t dB) {
  return std::powf(10.0f, 0.05f * dB);
}
// @param volume [0..1]
[[nodiscard]] inline float volumeTodB(float volume) {
  volume = glm::clamp(volume, 0.001f, 1.0f); // -60dB..0dB
  return 20.0f * std::log10f(volume);
}

} // namespace audio
