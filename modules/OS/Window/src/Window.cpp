#include "os/Window.hpp"
#include "os/Monitor.hpp"
#include "glm/ext/scalar_constants.hpp" // epsilon

namespace os {

Window::~Window() { _destroy(); }

Window::operator bool() const { return isOpen(); }

std::string_view Window::getCaption() const { return m_caption; }

const Window::PlatformData &Window::getNativeData() const { return m_native; }

//
// Helper:
//

float getAspectRatio(const Window &window) {
  const auto size = window.getExtent();
  return size.y > 0 ? float(size.x) / float(size.y) : glm::epsilon<float>();
}

void center(Window &window) {
  if (const auto monitor = getMonitor(window); monitor) {
    const auto monitorSize = monitor->mainArea.size;
    const auto windowSize = window.getExtent();
    window.setPosition((monitorSize / 2) - (windowSize / 2));
  }
}
glm::uvec2 getCenter(const Window &window) {
  return glm::uvec2{window.getPosition() + window.getExtent() / 2};
}

} // namespace os
