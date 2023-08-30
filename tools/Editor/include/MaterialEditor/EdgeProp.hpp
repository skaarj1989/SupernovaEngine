#pragma once

#include <cstdint>

// Hint to imnodes (whether to render a link or not).
enum class EdgeType {
  Explicit, // Draw a link.
  Implicit  // Internal connection (no link).
};

struct EdgeProp {
  int32_t id;
  EdgeType type;

  template <class Archive> void serialize(Archive &archive) {
    archive(id, type);
  }
};
