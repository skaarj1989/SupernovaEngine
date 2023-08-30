#pragma once

namespace os {

// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
enum class KeyCode {
  Backspace = 0x08,
  Tab = 0x09,
  Clear = 0x0C,  // Clear (on numpad)
  Return = 0x0D, // Enter
  Shift = 0x10,
  Control = 0x11, // Ctrl
  Menu = 0x12,    // Alt
  Pause = 0x13,   // Pause/Break
  Captial = 0x14, // Caps Lock
  Esc = 0x1B,     // Escape
  Space = 0x20,   // Space bar

  Prior = 0x21, // Page Up
  Next = 0x22,  // Page Down
  End = 0x23,
  Home = 0x24,

  Left = 0x25,
  Up = 0x26,
  Right = 0x27,
  Down = 0x28,

  Snapshot = 0x2C, // Print Screen
  Insert = 0x2D,   // Ins
  Delete = 0x2E,   // Del

  LWin = 0x5B,
  RWin = 0x5C,
  Apps = 0x5D,

  // clang-format off

  // -- Numeric:

  _0 = 0x30,
  _1, _2, _3, _4, _5, _6, _7, _8, _9,

  // -- Alpha:

  A = 0x41,
  B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

  // -- Numpad:

  Num0 = 0x60,
  Num1, Num2, Num3,
  Num4, Num5, Num6,
  Num7, Num8, Num9,
  Multiply, Add,
  Separator, Subtract, Decimal, Divide,

  // -- Func

  F1 = 0x70,
  F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

  // clang-format on

  NumLock = 0x90, // Num Lock
  SLock = 0x91,   // Scroll Lock

  LShift = 0xA0,   // Left Shift
  RShift = 0xA1,   // Right Shift
  LControl = 0xA2, // Left Control
  RControl = 0xA3, // Right Control
  LMenu = 0xA4,    // Left Alt
  RMenu = 0xA5     // Right Alt
};

} // namespace os
