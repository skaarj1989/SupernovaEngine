#pragma once

#include <limits>

struct SoundSettings {
  float pitch{1.0f};
  float gain{1.0f};
  float maxDistance{std::numeric_limits<float>::max()};
  float rollOffFactor{1.0f};
  float referenceDistance{1.0f};
  float minGain{0.0f};
  float maxGain{1.0f};
  bool directional{false};
  bool loop{false};
  bool playing{false};

  template <typename Archive> void serialize(Archive &archive) {
    archive(pitch, gain, maxDistance, rollOffFactor, referenceDistance, minGain,
            maxGain, directional, loop, playing);
  }
};
