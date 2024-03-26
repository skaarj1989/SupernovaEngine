#pragma once

#include "KeyCodes.hpp"
#include "glm/ext/vector_int2.hpp"
#include <cstdint>
#include <variant>

namespace os {

enum class KeyState { Undefined = -1, Up, Down };
struct KeyboardEvent {
  KeyState state;
  KeyCode keyCode;
  uint32_t charCode;
  bool repeated;
};
struct InputCharacterEvent {
  uint16_t c;
};

struct MouseMoveEvent {
  glm::ivec2 position;
};
enum class MouseButton { Undefined = -1, Left, Right, Middle, X1, X2 };
enum class MouseButtonState { Undefined = -1, Released, Pressed, DblClick };
struct MouseButtonEvent : MouseMoveEvent {
  MouseButtonState state;
  MouseButton button;
};
enum class MouseWheel {
  Undefined = -1,

  Horizontal,
  Vertical
};
struct MouseWheelEvent {
  MouseWheel wheel;
  float step;
};

using InputEvent =
  std::variant<MouseMoveEvent, MouseButtonEvent, MouseWheelEvent, KeyboardEvent,
               InputCharacterEvent>;

} // namespace os
