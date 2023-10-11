#pragma once

#include <chrono>
#include <utility> // to_underlying

using fsec = std::chrono::duration<float>;

namespace audio {

enum class NumChannels : uint8_t {
  Invalid = 0,
  Mono,
  Stereo,
  Surround1D,
  QuadSurround,
  Surround,
  Surround5_1,
  Surround6_1,
  Surround7_1,
};

struct ClipInfo {
  NumChannels numChannels{NumChannels::Invalid};
  uint8_t bitsPerSample{0};
  uint32_t sampleRate{0}; // = Frequency in Hz, samples per second.
  std::size_t numSamples{0};

  [[nodiscard]] constexpr auto blockAlign() const {
    return std::to_underlying(numChannels) * bitsPerSample / 8;
  }
  // @return Stride.
  [[nodiscard]] constexpr auto byteRate() const {
    return sampleRate * blockAlign();
  }

  // @returns Value in bytes.
  [[nodiscard]] constexpr auto dataSize() const {
    return numSamples * blockAlign();
  }
  [[nodiscard]] constexpr auto duration() const {
    return fsec{sampleRate > 0 ? static_cast<float>(numSamples) / sampleRate
                               : 0.0f};
  }
};

[[nodiscard]] constexpr auto align(const int32_t size, const ClipInfo &info) {
  return size - (size % info.blockAlign());
}
[[nodiscard]] constexpr auto calcBufferSize(const fsec duration,
                                            const ClipInfo &info) {
  return align(duration.count() * info.byteRate(), info);
}

} // namespace audio
