#include "os/InputSystem.hpp"
#include <Windows.h>

namespace os {

namespace {

[[nodiscard]] bool isCursorVisible() {
  CURSORINFO cursorInfo{.cbSize = sizeof(CURSORINFO)};
  ::GetCursorInfo(&cursorInfo);
  return cursorInfo.flags & CURSOR_SHOWING;
}
void showCursor() {
  while (::ShowCursor(TRUE) < 0)
    ;
}
void hideCursor() {
  while (::ShowCursor(FALSE) >= 0)
    ;
}

} // namespace

//
// InputSystem class:
//

InputSystem::InputSystem() : m_cursorVisible{os::isCursorVisible()} {}

void InputSystem::setMousePosition(glm::ivec2 v) {
  ::SetCursorPos(v.x, v.y);
  m_lastMousePosition = v;
}

glm::ivec2 InputSystem::getMousePosition() const {
  POINT p;
  ::GetCursorPos(&p);
  return {p.x, p.y};
}

void InputSystem::showCursor(bool show) {
  if (os::isCursorVisible() != show) {
    show ? os::showCursor() : os::hideCursor();
    m_cursorVisible = os::isCursorVisible();
  }
}

} // namespace os
