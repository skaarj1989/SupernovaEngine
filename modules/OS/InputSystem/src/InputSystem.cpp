#include "os/InputSystem.hpp"

namespace os {

namespace {

template <typename T, typename K>
[[nodiscard]] auto tryGet(const T &map, const K key) {
  const auto it = map.find(key);
  return it != map.cend() ? std::optional{it->second} : std::nullopt;
}

} // namespace

//
// InputSystem class:
//

void InputSystem::update() {
  auto currentMousePosition = getMousePosition();
  if (m_lastMousePosition) {
    m_mouseDelta = currentMousePosition - *m_lastMousePosition;
  }
  m_lastMousePosition = currentMousePosition;
}

glm::ivec2 InputSystem::getMouseDelta() const { return m_mouseDelta; }

bool InputSystem::isCursorVisible() const { return m_cursorVisible; }

bool InputSystem::isMouseDown(const MouseButton b) const {
  return tryGet(m_mouseButtons, b).value_or(false);
}
bool InputSystem::isMouseUp(const MouseButton b) const {
  return !isMouseDown(b);
}

bool InputSystem::isKeyDown(const KeyCode c) const {
  return tryGet(m_keys, c).value_or(false);
}
bool InputSystem::isKeyUp(const KeyCode c) const { return !isKeyDown(c); }

void InputSystem::notify(const MouseButtonEvent &evt) {
  assert(evt.state != MouseButtonState::Undefined);
  m_mouseButtons[evt.button] = evt.state == MouseButtonState::Pressed;
}
void InputSystem::notify(const KeyboardEvent &evt) {
  assert(evt.state != KeyState::Undefined);

  const auto pressed = evt.state == KeyState::Down;
  m_keys[evt.keyCode] = pressed;

  switch (evt.keyCode) {
    using enum os::KeyCode;

  case LShift:
  case RShift:
    m_keys[Shift] = pressed;
    break;
  case LControl:
  case RControl:
    m_keys[Control] = pressed;
    break;
  case LMenu:
  case RMenu:
    m_keys[Menu] = pressed;
    break;
  }
}

} // namespace os
