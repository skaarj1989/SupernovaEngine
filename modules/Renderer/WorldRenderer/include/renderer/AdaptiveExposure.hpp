#pragma once

namespace gfx {

struct AdaptiveExposure {
  float minLogLuminance{-10.0f};
  float maxLogLuminance{2.0f};
  float tau{1.1f};

  template <class Archive> void serialize(Archive &archive) {
    archive(minLogLuminance, maxLogLuminance, tau);
  }
};

} // namespace gfx
