#pragma once

#include "glm/ext/vector_int2.hpp"
#include <vector>
#include <optional>

namespace os {

class Window;

struct MonitorInfo {
  struct Rect {
    glm::ivec2 position{0}; // Top-left.
    glm::ivec2 size{0};     // Width, Height.
  };
  Rect mainArea;
  Rect workArea;
  float dpi{0.0f};
  uint32_t refreshRate{0};

  bool primary{false};
};
using Monitors = std::vector<MonitorInfo>;

[[nodiscard]] std::optional<MonitorInfo> getMonitor(const Window &);
[[nodiscard]] Monitors getMonitors();

} // namespace os
