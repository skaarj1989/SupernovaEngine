#pragma once

#include <cstdint>

struct CollisionLayer {
  uint8_t group{0}; // [0..kNumGroups-1]
  uint8_t mask{1 << 0};

  template <class Archive> void serialize(Archive &archive) {
    archive(group, mask);
  }

  constexpr static auto kNumGroups = 8;
};
