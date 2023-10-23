#include "os/Monitor.hpp"
#include "os/Window.hpp"
#include "os/X11.hpp"

// xrandr --query

namespace os {

namespace {

[[nodiscard]] auto convert(const xcb_randr_monitor_info_t &in) {
  constexpr auto kMillimetersPerInch = 25.4f;
  const auto xdpi = kMillimetersPerInch * in.width / in.width_in_millimeters;
  const auto ydpi = kMillimetersPerInch * in.height / in.height_in_millimeters;
  assert(xdpi == ydpi);

  MonitorInfo out{
    .mainArea =
      {
        .position = {in.x, in.y},
        .size = {in.width, in.height},
      },
    .dpi = xdpi / 96.0f,
    // TODO: .refreshRate
    .primary = in.primary == 1,
  };
  out.workArea = out.mainArea;
  return out;
}

[[nodiscard]] bool contains(const MonitorInfo::Rect &r, const glm::ivec2 &p) {
  return p.x >= r.position.x && p.y >= r.position.y &&
         p.x <= r.position.x + r.size.x && p.y <= r.position.y + r.size.y;
}

} // namespace

std::optional<MonitorInfo> getMonitor(const Window &window) {
  const auto monitors = getMonitors();
  const auto it = std::ranges::find_if(
    monitors, [p = window.getPosition()](const MonitorInfo &monitor) {
      return contains(monitor.mainArea, p);
    });
  return it != monitors.cend() ? std::make_optional(*it) : std::nullopt;
}
Monitors getMonitors() {
  std::vector<MonitorInfo> out;
  out.reserve(3);
  x11::iterateMonitors([&out](const xcb_randr_monitor_info_t &info) {
    out.emplace(info.primary ? out.begin() : out.end(), convert(info));
  });
  return out;
}

} // namespace os
