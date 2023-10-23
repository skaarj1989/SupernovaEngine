#include "os/Window.hpp"

#include "os/X11.hpp"
#include <xcb/xcb_event.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>

#include <algorithm> // copy
#include <format>

namespace os {

namespace {

void registerWindow(const Window *window) {
  assert(window && window->isOpen());
  x11::setUserData(window->getNativeData().id,
                   std::bit_cast<uintptr_t>(window));
}
[[nodiscard]] auto *get(const xcb_window_t window) {
  return std::bit_cast<Window *>(x11::getUserData(window));
}

[[nodiscard]] auto getState(const xcb_window_t window) {
  if (const auto states = x11::getWindowStates(window); !states.empty()) {
    if (x11::isMinimized(states)) {
      return Window::State::Minimized;
    } else if (x11::isMaximized(states)) {
      return Window::State::Maximized;
    }
  }
  return Window::State::Windowed;
}

bool updatePosition(const xcb_window_t window, glm::ivec2 &position) {
  if (const auto v = x11::getPosition(window); position != v) {
    position = v;
    return true;
  }
  return false;
}
bool updateExtent(const xcb_window_t window, glm::ivec2 &clientExtent) {
  if (const auto v = x11::getExtent(window); clientExtent != v) {
    clientExtent = v;
    return true;
  }
  return false;
}

[[nodiscard]] std::optional<os::MouseButton>
convertMouseButton(const xcb_button_t id) {
  switch (id) {
  case 1:
    return os::MouseButton::Left;
  case 2:
    return os::MouseButton::Middle;
  case 3:
    return os::MouseButton::Right;
  }
  return std::nullopt;
}
[[nodiscard]] MouseWheel getMouseWheel(const xcb_button_t id) {
  using enum MouseWheel;
  switch (id) {
  case 4:
  case 5:
    return Vertical;
  case 6:
  case 7:
    return Horizontal;
  }
  assert(false);
  return Undefined;
}
[[nodiscard]] std::optional<KeyCode> convertKey(const xcb_keycode_t kc) {
  switch (x11::getKeySym(kc)) {
    using enum KeyCode;

  // clang-format off
  case XK_BackSpace: return Backspace;
  case XK_Tab: return Tab;
  case XK_Clear: return Clear;

  case XK_Return: case XK_KP_Enter: return Return;
  case XK_Pause: return Pause;
  case XK_Caps_Lock: return Captial;
  case XK_Escape: return Esc;
  case XK_space:  return Space;

  case XK_Prior: return Prior;
  case XK_Next: return Next;
  case XK_End: return End;
  case XK_Home: return Home;
    
  case XK_Left: return Left;
  case XK_Up: return Up;
  case XK_Right: return Right;
  case XK_Down: return Down;

  case XK_Print: return Snapshot;
  case XK_Insert: return Insert;
  case XK_Delete: case XK_KP_Delete: return Delete;

  case XK_Hyper_L: return LWin;
  case XK_Hyper_R: return RWin;

  case XK_0: return _0;
  case XK_1: return _1;
  case XK_2: return _2;
  case XK_3: return _3;
  case XK_4: return _4;
  case XK_5: return _5;
  case XK_6: return _6;
  case XK_7: return _7;
  case XK_8: return _8;
  case XK_9: return _9;

  case XK_A: case XK_a: return A;
  case XK_B: case XK_b: return B;
  case XK_C: case XK_c: return C;
  case XK_D: case XK_d: return D;
  case XK_E: case XK_e: return E;
  case XK_F: case XK_f: return F;
  case XK_G: case XK_g: return G;
  case XK_H: case XK_h: return H;
  case XK_I: case XK_i: return I;
  case XK_J: case XK_j: return J;
  case XK_K: case XK_k: return K;
  case XK_L: case XK_l: return L;
  case XK_M: case XK_m: return M;
  case XK_N: case XK_n: return N;
  case XK_O: case XK_o: return O;
  case XK_P: case XK_p: return P;
  case XK_Q: case XK_q: return Q;
  case XK_R: case XK_r: return R;
  case XK_S: case XK_s: return S;
  case XK_T: case XK_t: return T;
  case XK_U: case XK_u: return U;
  case XK_V: case XK_v: return V;
  case XK_W: case XK_w: return W;
  case XK_X: case XK_x: return X;
  case XK_Y: case XK_y: return Y;
  case XK_Z: case XK_z: return Z;

  case XK_KP_0: case XK_KP_Insert: return Num0;
  case XK_KP_1: case XK_KP_End: return Num1;
  case XK_KP_2: case XK_KP_Down: return Num2;
  case XK_KP_3: case XK_KP_Page_Down: return Num3;
  case XK_KP_4: case XK_KP_Left: return Num4;
  case XK_KP_5: case XK_KP_Begin: return Num5;
  case XK_KP_6: case XK_KP_Right: return Num6;
  case XK_KP_7: case XK_KP_Home: return Num7;
  case XK_KP_8: case XK_KP_Up: return Num8;
  case XK_KP_9: case XK_KP_Page_Up: return Num9;

  case XK_KP_Multiply: return Multiply;
  case XK_KP_Add: return Add;
  case XK_KP_Separator: return Separator;
  case XK_KP_Subtract: return Subtract;
  case XK_KP_Decimal: return Decimal;
  case XK_KP_Divide: return Divide;

  case XK_F1: return F1;
  case XK_F2: return F2;
  case XK_F3: return F3;
  case XK_F4: return F4;
  case XK_F5: return F5;
  case XK_F6: return F6;
  case XK_F7: return F7;
  case XK_F8: return F8;
  case XK_F9: return F9;
  case XK_F10: return F10;
  case XK_F11: return F11;
  case XK_F12: return F12;

  case XK_Num_Lock: return NumLock;
  case XK_Scroll_Lock: return SLock;

  case XK_Shift_L: return LShift;
  case XK_Shift_R: return RShift;
  case XK_Control_L: return LControl;
  case XK_Control_R: return RControl;
  case XK_Alt_L: return LMenu;
  case XK_Alt_R: return RMenu;
  };
  // clang-format on

  return std::nullopt;
}

} // namespace

//
// Window class:
//

Window::Window(Window &&other) noexcept
    : entt::emitter<Window>{std::move(other)}, m_native{other.m_native},
      m_frame{std::move(other.m_frame)}, m_clientOrigin{other.m_clientOrigin},
      m_clientExtent{other.m_clientExtent}, m_state{other.m_state},
      m_focused{other.m_focused}, m_caption{std::move(other.m_caption)},
      m_alpha{other.m_alpha} {
  other.m_native.id = XCB_WINDOW_NONE;
  registerWindow(this);
}

Window &Window::operator=(Window &&rhs) noexcept {
  assert(m_native.id == XCB_WINDOW_NONE);

  if (this != &rhs) {
    entt::emitter<Window>::operator=(std::move(rhs));

    std::swap(m_native, rhs.m_native);

    std::swap(m_frame, rhs.m_frame);
    std::swap(m_clientOrigin, rhs.m_clientOrigin);
    std::swap(m_clientExtent, rhs.m_clientExtent);
    m_state = rhs.m_state;
    m_focused = rhs.m_focused;

    m_caption = std::move(rhs.m_caption);
    m_alpha = rhs.m_alpha;

    registerWindow(this);
  }
  return *this;
}

Window &Window::setPosition(const Position position) {
  assert(isOpen());
  x11::setPosition(m_native.id, position);
  x11::flush();
  return *this;
}
Window &Window::setExtent(const Extent extent) {
  assert(isOpen());
  x11::setExtent(m_native.id, extent - m_frame);
  x11::flush();
  return *this;
}
Window &Window::setAlpha(const float a) {
  m_alpha = std::clamp(a, 0.0f, 1.0f);
  x11::setAlpha(m_native.id, m_alpha);
  x11::flush();
  return *this;
}

Window &Window::setCaption(const std::string_view caption) {
  m_caption = caption;
  x11::setCaption(m_native.id, m_caption);
  x11::flush();
  return *this;
}

glm::ivec2 Window::getPosition(const Area area) const {
  assert(isOpen());
  return area == Area::Client ? m_clientOrigin : m_clientOrigin - m_frame;
}
glm::ivec2 Window::getExtent(const Area area) const {
  assert(isOpen());
  return area == Area::Client ? m_clientExtent : m_clientExtent + m_frame;
}

bool Window::isOpen() const { return m_native.id != XCB_WINDOW_NONE; }
Window::State Window::getState() const {
  assert(isOpen());
  return m_state;
}
bool Window::hasFocus() const {
  assert(isOpen());
  return m_focused;
}

Window &Window::show() {
  assert(isOpen());
  x11::show(m_native.id);
  x11::flush();
  return *this;
}
Window &Window::hide() {
  assert(isOpen());
  x11::hide(m_native.id);
  x11::flush();
  return *this;
}

Window &Window::minimize() {
  assert(isOpen());
  x11::minimize(m_native.id);
  x11::flush();
  return *this;
}
Window &Window::maximize() {
  assert(isOpen());
  x11::maximize(m_native.id);
  x11::flush();
  return *this;
}

Window &Window::focus() {
  assert(isOpen());
  x11::focus(m_native.id);
  x11::flush();
  return *this;
}

void Window::close() {
  assert(isOpen());
  x11::close(m_native.id);
  x11::flush();
}

//
// (private):
//

Window::Window(const Window *parent, const Position position,
               const Extent extent, const float alpha,
               const std::string_view caption) {
  m_native.id =
    x11::createWindow(parent ? parent->m_native.id : XCB_WINDOW_NONE, position,
                      extent, alpha, caption);
  registerWindow(this);
  m_native.connection = x11::getConnection();

  m_clientOrigin = position;
  m_clientExtent = extent;
  m_alpha = alpha;
  m_caption = caption;
}

void Window::_destroy() {
  if (m_native.id != XCB_WINDOW_NONE) {
    x11::destroyWindow(m_native.id);
    x11::flush();
    m_native.id = XCB_WINDOW_NONE;
  }
}

//
// Event dispatcher:
//

uint32_t pollEvents() {
  uint32_t count{0};

#define XCB_UNDERLYING_EVENT(name)                                             \
  reinterpret_cast<xcb_##name##_event_t *>(genericEvent.get())

  x11::SmartGenericEvent genericEvent{nullptr, nullptr};

  x11::flush();
  while (genericEvent = x11::pollForEvent()) {
    switch (XCB_EVENT_RESPONSE_TYPE(genericEvent)) {
    case XCB_NONE:
      assert(false);
      break;

    case XCB_CONFIGURE_NOTIFY: { // XCB_EVENT_MASK_STRUCTURE_NOTIFY
      const auto *evt = XCB_UNDERLYING_EVENT(configure_notify);
      if (auto *window = get(evt->window); window) {
        updatePosition(evt->window, window->m_clientOrigin);
        if (window->contains<MoveWindowEvent>()) {
          window->publish(MoveWindowEvent{.position = window->getPosition()});
        }
      }
    } break;

    case XCB_CLIENT_MESSAGE: {
      const auto *evt = XCB_UNDERLYING_EVENT(client_message);
      if (auto *window = get(evt->window); window) {
        if (x11::isCloseMessage(evt)) {
          if (window->contains<CloseWindowEvent>()) {
            window->publish(CloseWindowEvent{});
          } else {
            window->_destroy();
          }
        }
      }
    } break;

    case XCB_FOCUS_IN:
    case XCB_FOCUS_OUT: { // XCB_EVENT_MASK_FOCUS_CHANGE
      const auto *evt = XCB_UNDERLYING_EVENT(focus_in);
      if (auto *window = get(evt->event); window) {
        window->m_focused = evt->response_type == XCB_FOCUS_IN;
      }
    } break;

    case XCB_PROPERTY_NOTIFY: { // XCB_EVENT_MASK_PROPERTY_CHANGE
      const auto *evt = XCB_UNDERLYING_EVENT(property_notify);
      if (auto *window = get(evt->window); window) {
        switch (x11::getChangedPropertyID(evt)) {
        case x11::PropertyID::State:
          if (const auto newState = getState(evt->window);
              window->m_state != newState) {
            // Unfortunately, X server does not notify about window resize
            // when transitioning to/from minimized state.
            if (newState == Window::State::Minimized) {
              if (window->contains<MoveWindowEvent>()) {
                window->publish(MoveWindowEvent{.position = {-32000, -32000}});
              }
              if (window->contains<ResizeWindowEvent>()) {
                window->publish(ResizeWindowEvent{.size = {0, 0}});
              }
            } else if (window->m_state == Window::State::Minimized) {
              if (window->contains<MoveWindowEvent>()) {
                window->publish(
                  MoveWindowEvent{.position = window->getPosition()});
              }
              if (window->contains<ResizeWindowEvent>()) {
                window->publish(ResizeWindowEvent{.size = window->getExtent()});
              }
            }
            window->m_state = newState;
          }
          break;

        case x11::PropertyID::FrameExtents:
          window->m_frame = x11::getFrameExtents(evt->window);
          break;
        }
      }
    } break;

    case XCB_EXPOSE: { // XCB_EVENT_MASK_EXPOSURE
      const auto *evt = XCB_UNDERLYING_EVENT(expose);
      if (auto *window = get(evt->window); window) {
        updateExtent(evt->window, window->m_clientExtent);
        if (window->contains<ResizeWindowEvent>()) {
          window->publish(ResizeWindowEvent{.size = window->getExtent()});
        }
      }
    } break;

    case XCB_KEY_PRESS:     // XCB_EVENT_MASK_KEY_PRESS
    case XCB_KEY_RELEASE: { // XCB_EVENT_MASK_KEY_RELEASE
      const auto *evt = XCB_UNDERLYING_EVENT(key_press);
      if (auto *window = get(evt->event); window) {
        if (const auto kc = convertKey(evt->detail);
            kc && window->contains<KeyboardEvent>()) {
          window->publish(KeyboardEvent{
            .state = evt->response_type == XCB_KEY_PRESS ? KeyState::Down
                                                         : KeyState::Up,
            .keyCode = *kc,
          });
        }
        if (evt->response_type == XCB_KEY_PRESS) {
          if (auto c = x11::characterLookup(evt->detail, evt->state);
              c && window->contains<InputCharacterEvent>()) {
            window->publish(InputCharacterEvent{static_cast<uint16_t>(*c)});
          }
        }
      }
    } break;

    case XCB_MOTION_NOTIFY: { // XCB_EVENT_MASK_POINTER_MOTION
      const auto *evt = XCB_UNDERLYING_EVENT(motion_notify);
      if (auto *window = get(evt->event);
          window && window->contains<MouseMoveEvent>()) {
        //  root_{x/y} = relative to the screen.
        // event_{x/y} = relative to the window.
        window->publish(
          MouseMoveEvent{.position = {evt->event_x, evt->event_y}});
      }
    } break;

    case XCB_BUTTON_PRESS:     // XCB_EVENT_MASK_BUTTON_PRESS
    case XCB_BUTTON_RELEASE: { // XCB_EVENT_MASK_BUTTON_RELEASE
      const auto *evt = XCB_UNDERLYING_EVENT(button_press);
      if (auto *window = get(evt->event); window) {
        if (x11::isMouseWheelEvent(evt)) {
          if (evt->response_type == XCB_BUTTON_PRESS &&
              window->contains<MouseWheelEvent>()) {
            window->publish(MouseWheelEvent{
              .wheel = getMouseWheel(evt->detail),
              .step = evt->detail % 2 ? -1.0f : 1.0f,
            });
          }
        } else {
          if (auto btn = convertMouseButton(evt->detail);
              btn && window->contains<MouseButtonEvent>()) {
            window->publish(MouseButtonEvent{
              glm::ivec2{evt->event_x, evt->event_y},
              evt->response_type == XCB_BUTTON_PRESS
                ? MouseButtonState::Pressed
                : MouseButtonState::Released,
              *btn,
            });
          }
        }
      }
    } break;

    case XCB_SELECTION_REQUEST:
      x11::handleSelectionRequest(XCB_UNDERLYING_EVENT(selection_request));
      break;
    }
    x11::flush();

    ++count;
  }

#undef XCB_UNDERLYING_EVENT

  return count;
}

} // namespace os
