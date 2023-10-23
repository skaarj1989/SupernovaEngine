#include "os/InputSystem.hpp"
#include "os/X11.hpp"

namespace os {

InputSystem::InputSystem() : m_cursorVisible{x11::isCursorVisible()} {}

void InputSystem::setMousePosition(const glm::ivec2 v) {
  x11::setCursorPosition(v);
  x11::flush();
  m_lastMousePosition = v;
}
glm::ivec2 InputSystem::getMousePosition() const {
  return x11::getCursorPosition();
}

void InputSystem::showCursor(const bool show) {
  if (x11::isCursorVisible() != show) {
    show ? x11::showCursor() : x11::hideCursor();
    m_cursorVisible = x11::isCursorVisible();
  }
}

} // namespace os
