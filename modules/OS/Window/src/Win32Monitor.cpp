#include "os/Monitor.hpp"
#include "os/Window.hpp"
#include <ShellScalingApi.h> // GetDpiForMonitor

#pragma comment(lib, "shcore.lib")

namespace os {

namespace {

[[nodiscard]] auto convert(const RECT &rect) {
  return MonitorInfo::Rect{
    .position = {rect.left, rect.top},
    .size =
      {
        rect.right - rect.left,
        rect.bottom - rect.top,
      },
  };
}

[[nodiscard]] std::optional<MonitorInfo> getMonitorInfo(HMONITOR hMonitor) {
  MONITORINFOEX info{{.cbSize = sizeof(MONITORINFOEX)}};
  if (!::GetMonitorInfo(hMonitor, &info)) return std::nullopt;

  DEVMODE dm{.dmSize = sizeof(DEVMODE)};
  if (!::EnumDisplaySettings(info.szDevice, ENUM_CURRENT_SETTINGS, &dm))
    return std::nullopt;

  UINT xdpi{96};
  UINT ydpi{96};
  ::GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
  assert(xdpi == ydpi);

  return MonitorInfo{
    .mainArea = convert(info.rcMonitor),
    .workArea = convert(info.rcWork),
    .dpi = float(xdpi) / 96.0f,
    .refreshRate = dm.dmDisplayFrequency,
    .primary = bool(info.dwFlags & MONITORINFOF_PRIMARY),
  };
}

} // namespace

std::optional<MonitorInfo> getMonitor(const Window &window) {
  auto hMonitor =
    ::MonitorFromWindow(window.getNativeData().hWnd, MONITOR_DEFAULTTONEAREST);
  return getMonitorInfo(hMonitor);
}
Monitors getMonitors() {
  Monitors monitors;
  ::EnumDisplayMonitors(
    nullptr, nullptr,
    [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) {
      assert(lParam != 0);
      auto monitorInfo = getMonitorInfo(hMonitor);
      if (!monitorInfo) return FALSE;

      auto &dst = *std::bit_cast<Monitors *>(lParam);
      dst.emplace(monitorInfo->primary ? dst.begin() : dst.end(),
                  std::move(*monitorInfo));
      return TRUE;
    },
    std::bit_cast<LPARAM>(&monitors));

  return monitors;
}

} // namespace os
