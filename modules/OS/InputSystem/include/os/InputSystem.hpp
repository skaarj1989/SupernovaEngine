#pragma once

#include "os/InputEvents.hpp"
#include <optional>
#include <unordered_map>

namespace os {

class InputSystem final {
public:
  InputSystem();

  void update();

  // -- Mouse:

  void setMousePosition(const glm::ivec2);

  [[nodiscard]] glm::ivec2 getMousePosition() const;
  [[nodiscard]] glm::ivec2 getMouseDelta() const;

  void showCursor(const bool);
  bool isCursorVisible() const;

  bool isMouseDown(const MouseButton) const;
  bool isMouseUp(const MouseButton) const;

  // -- Keyboard:

  bool isKeyDown(const KeyCode) const;
  bool isKeyUp(const KeyCode) const;

  void notify(const MouseButtonEvent &);
  void notify(const KeyboardEvent &);

private:
  bool m_cursorVisible{true};

  std::optional<glm::ivec2> m_lastMousePosition;
  glm::ivec2 m_mouseDelta{0};
  std::unordered_map<MouseButton, bool> m_mouseButtons;

  std::unordered_map<KeyCode, bool> m_keys;
  std::unordered_map<KeyCode, bool> m_previousKeys;
};

} // namespace os
