#include "os/Window.hpp"
#include <shellapi.h> // Drag & Drop
#include <windowsx.h> // GET_{X|Y}_LPARAM
#include <dwmapi.h>   // DwmSetWindowAttribute

#include "tracy/Tracy.hpp"

#pragma comment(lib, "dwmapi.lib")

namespace os {

namespace {

void setUserData(HWND hWnd, const Window *window) {
  assert(::IsWindow(hWnd) && window);
  ::SetWindowLongPtr(hWnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(window));
}
[[nodiscard]] auto getUserData(HWND hWnd) {
  assert(::IsWindow(hWnd));
  return std::bit_cast<Window *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

void setDarkMode(HWND hWnd) {
  assert(::IsWindow(hWnd));

  // https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute

#if _WIN32_WINNT <= _WIN32_WINNT_WIN10
  constexpr auto DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
#endif
  const BOOL value{TRUE};
  ::DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value,
                          sizeof(value));

#if _WIN32_WINNT > _WIN32_WINNT_WIN10
  constexpr COLORREF kDarkColor = 0x00505050;
  ::DwmSetWindowAttribute(hWnd, DWMWA_BORDER_COLOR, &kDarkColor,
                          sizeof(kDarkColor));
  ::DwmSetWindowAttribute(hWnd, DWMWA_CAPTION_COLOR, &kDarkColor,
                          sizeof(kDarkColor));
#endif
}

void retrieveBounds(HWND hWnd, RECT &windowRect, SIZE &clientSize) {
  assert(::IsWindow(hWnd));

  ::GetWindowRect(hWnd, &windowRect);
  RECT clientRect{};
  ::GetClientRect(hWnd, &clientRect);
  clientSize = {clientRect.right, clientRect.bottom};
}

[[nodiscard]] MouseButton extractMouseButton(UINT uMsg, WPARAM wParam) {
  using enum MouseButton;
  switch (uMsg) {
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
    return Left;

  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
    return Right;

  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
    return Middle;

  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
    switch (GET_XBUTTON_WPARAM(wParam)) {
    case XBUTTON1:
      return X1;
    case XBUTTON2:
      return X2;
    }
    break;
  }
  assert(false);
  return Undefined;
}
[[nodiscard]] MouseWheel extractMouseWheel(UINT uMsg) {
  using enum MouseWheel;
  switch (uMsg) {
  case WM_MOUSEWHEEL:
    return Vertical;
  case WM_MOUSEHWHEEL:
    return Horizontal;
  }
  assert(false);
  return Undefined;
}
[[nodiscard]] float extractMouseWheelStep(WPARAM wParam) {
  return float(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
}

}; // namespace

//
// Window class:
//

Window::Window(Window &&other) noexcept
    : entt::emitter<Window>{std::move(other)},
      m_native{std::move(other.m_native)}, m_style{other.m_style},
      m_exStyle{other.m_exStyle}, m_bounds{std::move(other.m_bounds)},
      m_clientSize{other.m_clientSize}, m_caption{std::move(other.m_caption)},
      m_alpha{other.m_alpha} {
  setUserData(m_native.hWnd, this);
  other.m_native = {};
}

Window &Window::operator=(Window &&rhs) noexcept {
  assert(m_native.hWnd == nullptr);

  if (this != &rhs) {
    entt::emitter<Window>::operator=(std::move(rhs));

    std::swap(m_native, rhs.m_native);
    setUserData(m_native.hWnd, this);

    m_style = rhs.m_style;
    m_exStyle = rhs.m_exStyle;

    m_bounds = std::move(rhs.m_bounds);
    m_clientSize = rhs.m_clientSize;

    m_caption = std::move(rhs.m_caption);
    m_alpha = rhs.m_alpha;
  }
  return *this;
}

Window &Window::setPosition(const Position position) {
  assert(isOpen());
  ::SetWindowPos(m_native.hWnd, nullptr, position.x, position.y, -1, -1,
                 SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
  return *this;
}
Window &Window::setExtent(const Extent extent) {
  assert(isOpen());
  ::SetWindowPos(m_native.hWnd, nullptr, -1, -1, extent.x, extent.y,
                 SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
  return *this;
}
Window &Window::setAlpha(const float a) {
  assert(isOpen());
  if (m_alpha != a) {
    ::SetLayeredWindowAttributes(m_native.hWnd, 0, BYTE(255 * a), LWA_ALPHA);
    m_alpha = a;
  }
  return *this;
}

Window &Window::setCaption(const std::string_view caption) {
  assert(isOpen());
  if (caption != m_caption) {
    m_caption = caption;
    ::SetWindowText(m_native.hWnd, m_caption.c_str());
  }
  return *this;
}

Window::Position Window::getPosition(const Area area) const {
  assert(isOpen());
  if (area == Area::Client) {
    POINT pt{0, 0};
    ::ClientToScreen(m_native.hWnd, &pt);
    return {pt.x, pt.y};
  } else {
    return {m_bounds.left, m_bounds.top};
  }
}
Window::Extent Window::getExtent(const Area area) const {
  assert(isOpen());
  return area == Area::Client ? Extent{m_clientSize.cx, m_clientSize.cy}
                              : Extent{
                                  m_bounds.right - m_bounds.left,
                                  m_bounds.bottom - m_bounds.top,
                                };
}
float Window::getAlpha() const { return m_alpha; }

bool Window::isOpen() const { return ::IsWindow(m_native.hWnd); }
Window::State Window::getState() const {
  using enum State;
  if (::IsIconic(m_native.hWnd)) {
    return Minimized;
  } else if (::GetWindowLong(m_native.hWnd, GWL_STYLE) & WS_MAXIMIZE) {
    return Maximized;
  }
  return Windowed;
}
bool Window::hasFocus() const {
  assert(isOpen());
  return ::GetForegroundWindow() == m_native.hWnd;
}

Window &Window::show() {
  assert(isOpen());
  ::ShowWindow(m_native.hWnd, m_style & WS_POPUP ? SW_SHOWNA : SW_SHOW);
  return *this;
}
Window &Window::hide() {
  assert(isOpen());
  ::ShowWindow(m_native.hWnd, SW_HIDE);
  return *this;
}

Window &Window::minimize() {
  assert(isOpen());
  ::ShowWindow(m_native.hWnd, SW_MINIMIZE);
  return *this;
}
Window &Window::maximize() {
  assert(isOpen());
  ::ShowWindow(m_native.hWnd, SW_MAXIMIZE);
  return *this;
}

Window &Window::focus() {
  assert(isOpen());
  ::BringWindowToTop(m_native.hWnd);
  ::SetForegroundWindow(m_native.hWnd);
  ::SetFocus(m_native.hWnd);
  return *this;
}

void Window::close() {
  assert(isOpen());
  ::SendMessage(m_native.hWnd, WM_CLOSE, 0, 0);
}

//
// (private):
//

Window::Window(const Window *parent, const Position position,
               const Extent extent, const float alpha,
               const std::string_view caption) {
  assert(!parent || parent->isOpen());

  static const WNDCLASSEX windowClass{
    .cbSize = sizeof(WNDCLASSEX),
    .style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC,
    .lpfnWndProc = Window::_wndProcRouter,
    .hInstance = ::GetModuleHandle(nullptr),
    .hIcon = ::LoadIcon(nullptr, IDI_APPLICATION),
    .hCursor = ::LoadCursor(nullptr, IDC_ARROW),
    .hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)),
    .lpszClassName = "Win32Window",
    .hIconSm = windowClass.hIcon,
  };
  static const ATOM id = ::RegisterClassEx(&windowClass);
  assert(id != 0);

  RECT rect{
    position.x,
    position.y,
    position.x + extent.x,
    position.y + extent.y,
  };
  const DWORD style{parent ? WS_POPUP : WS_OVERLAPPEDWINDOW};
  const DWORD exStyle{WS_EX_APPWINDOW | WS_EX_LAYERED};
  if (style & WS_POPUP) ::AdjustWindowRectEx(&rect, style, FALSE, exStyle);

  // clang-format off
  ::CreateWindowEx(
    exStyle,
    MAKEINTATOM(id),        // className
    caption.data(),
    style,
    rect.left,              // x
    rect.top,               // y
    rect.right - rect.left, // width
    rect.bottom - rect.top, // height
    parent ? parent->m_native.hWnd : HWND_DESKTOP,
    nullptr,                // hMenu
    windowClass.hInstance,
    this
  );
  // clang-format on

  m_style = style;
  m_exStyle = exStyle;
  m_caption = caption;

  retrieveBounds(m_native.hWnd, m_bounds, m_clientSize);

  setDarkMode(m_native.hWnd);
  setAlpha(alpha);
}

LRESULT Window::_wndProcRouter(HWND hWnd, UINT uMsg, WPARAM wParam,
                               LPARAM lParam) {
  Window *window{nullptr};
  if (uMsg == WM_NCCREATE) {
    const auto createStruct = std::bit_cast<LPCREATESTRUCT>(lParam);
    window = static_cast<Window *>(createStruct->lpCreateParams);
    setUserData(hWnd, window);

    window->m_native = {
      .hInstance = createStruct->hInstance,
      .hWnd = hWnd,
      .hDC = ::GetDC(hWnd),
    };

    ::DragAcceptFiles(hWnd, TRUE);
  } else {
    window = getUserData(hWnd);
  }

  return window ? window->_wndProc(hWnd, uMsg, wParam, lParam)
                : ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT Window::_wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  assert(m_native.hWnd == hWnd);
  ZoneScopedN("OS::WindowProc");

  switch (uMsg) {
  case WM_MOVE:
    retrieveBounds(hWnd, m_bounds, m_clientSize);
    if (contains<MoveWindowEvent>()) {
      publish(MoveWindowEvent{getPosition()});
      return 0;
    }
    break;

  case WM_SIZE:
    retrieveBounds(hWnd, m_bounds, m_clientSize);
    if (contains<ResizeWindowEvent>()) {
      publish(ResizeWindowEvent{getExtent()});
      return 0;
    }
    break;

  case WM_CLOSE:
    // Sent as a signal that a window or an application should terminate.
    if (contains<CloseWindowEvent>()) {
      publish(CloseWindowEvent{});
      return 0;
    }
    // else, default processing, then proceed to WM_DESTROY.
    break;

  case WM_DESTROY:
    // Sent when a window is being destroyed. It is sent to the window
    // procedure of the window being destroyed after the window is removed
    // from the screen.
    ::ReleaseDC(hWnd, m_native.hDC);
    m_native.hDC = nullptr;
    break;

  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP:
    if (contains<KeyboardEvent>()) {
      union {
        LPARAM lParam;
        struct {
          uint32_t repeatCount : 16;
          uint32_t scanCode : 8; // The value depends on the OEM.
          // Indicates whether the key is an extended key, such
          // as the right-hand ALT and CTRL keys that appear
          // on an enhanced 101- or 102-key keyboard. The value
          // is 1 if it is an extended key; otherwise, it is 0.
          uint32_t extended : 1;
          uint32_t reserved : 4; // Do not use
          // The context code. The value is 1 if the ALT key
          // is down while the key is pressed; it is 0 if the
          // WM_SYSKEYDOWN message is posted to the active
          // window because no window has the keyboard focus.
          uint32_t contextCode : 1;
          // The previous key state. The value is 1 if the key is down before
          // the message is sent, or it is 0 if the key is up.
          uint32_t prevState : 1;
          // The transition state. The value is always 0 for
          // WM_{KEY|SYSKEY}DOWN and 1 for WM_{KEY|SYSKEY}UP
          uint32_t transitionState : 1;
        };
      } keyState{.lParam = lParam};

      auto keyCode = int32_t(wParam);
      switch (keyCode) {
      case VK_SHIFT:
        keyCode = ::MapVirtualKey(keyState.scanCode, MAPVK_VSC_TO_VK_EX);
        break;
      case VK_CONTROL:
        keyCode = keyState.extended ? VK_RCONTROL : VK_LCONTROL;
        break;
      case VK_MENU:
        keyCode = keyState.extended ? VK_RMENU : VK_LMENU;
        break;
      }
      const uint32_t charCode{::MapVirtualKey(keyCode, MAPVK_VK_TO_CHAR)};

      publish(KeyboardEvent{
        .state = keyState.transitionState ? KeyState::Up : KeyState::Down,
        .keyCode = KeyCode(keyCode),
        .charCode = charCode,
        .repeated = keyState.repeatCount > 0,
      });
      return 0;
    }
    break;

  case WM_CHAR:
    if (contains<InputCharacterEvent>() && (wParam > 0 && wParam < 0x10000)) {
      publish(InputCharacterEvent{uint16_t(wParam)});
      return 0;
    }
    break;

  case WM_MOUSEACTIVATE:
    if (m_style & WS_POPUP) return MA_NOACTIVATE;
    break;

  case WM_MOUSEMOVE:
    if (contains<MouseMoveEvent>()) {
      publish(MouseMoveEvent{
        glm::ivec2{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)},
      });
      return 0;
    }
    break;

  case WM_LBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_XBUTTONDOWN:
    if (::GetCapture() == HWND_DESKTOP) ::SetCapture(m_native.hWnd);
    if (contains<MouseButtonEvent>()) {
      publish(MouseButtonEvent{
        glm::ivec2{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)},
        MouseButtonState::Pressed,
        extractMouseButton(uMsg, wParam),
      });
      return 0;
    }
    break;

  case WM_LBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MBUTTONUP:
  case WM_XBUTTONUP:
    if (::GetCapture() == m_native.hWnd) ::ReleaseCapture();
    if (contains<MouseButtonEvent>()) {
      publish(MouseButtonEvent{
        glm::ivec2{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)},
        MouseButtonState::Released,
        extractMouseButton(uMsg, wParam),
      });
      return 0;
    }
    break;

  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL:
    if (contains<MouseWheelEvent>()) {
      publish(MouseWheelEvent{
        .wheel = extractMouseWheel(uMsg),
        .step = extractMouseWheelStep(wParam),
      });
      return 0;
    }
    break;

  case WM_DROPFILES:
    if (contains<DropFilesEvent>()) {
      const auto hDrop = std::bit_cast<HDROP>(wParam);
      POINT dropPoint;
      ::DragQueryPoint(hDrop, &dropPoint);
      const auto numFiles = ::DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);

      std::set<std::filesystem::path> filenames;
      constexpr auto kBufferSize = 256;
      for (std::decay_t<decltype(numFiles)> i{0}; i < numFiles; ++i) {
        const auto filename = std::make_unique<char[]>(kBufferSize);
        ::DragQueryFile(hDrop, i, filename.get(), kBufferSize);
        filenames.emplace(filename.get());
      }
      ::DragFinish(hDrop);

      publish(DropFilesEvent{
        .position = glm::ivec2{dropPoint.x, dropPoint.y},
        .filenames = std::move(filenames),
      });
      return 0;
    }
    break;
  }

  return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Window::_destroy() {
  if (::IsWindow(m_native.hWnd)) {
    if (::GetCapture() == m_native.hWnd) {
      // Transfer capture so if we started dragging from a window that later
      // disappears, we'll still receive the MOUSEUP event.
      ::ReleaseCapture();
      ::SetCapture(::GetParent(m_native.hWnd));
    }
    ::DestroyWindow(m_native.hWnd);
    m_native.hWnd = nullptr;
  }
}

//
// Utility:
//

uint32_t pollEvents() {
  ZoneScopedN("OS::PollEvents");
  MSG msg{};
  uint32_t count{0};
  while (::PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);

    ++count;
  }
  return count;
}

} // namespace os
