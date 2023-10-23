#include "os/X11.hpp"

#define _X11_XCB 1

#if _X11_XCB
// https://xcb.freedesktop.org/MixingCalls/
#  include <X11/Xlib-xcb.h>
#  include <X11/Xutil.h> // XStringLookup
#endif

#include <xcb/xcb_event.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xfixes.h>
#include <xcb/xcb_keysyms.h>

#include <string>
#include <utility> // to_underlying
#include <ranges>

/*
libxcb-util-dev     -> X11::xcb_util
libxcb-icccm4-dev   -> X11::xcb_icccm
libxcb-ewmh-dev     -> X11::xcb_ewmh
libxcb-xfixes0-dev  -> X11::xcb_xfixes
libxcb-keysyms1-dev -> X11::xcb_keysyms
libxcb-randr0-dev   -> X11::xcb_randr
libx11-xcb-dev      -> X11::X11_xcb
*/

namespace x11 {

namespace {

#if _X11_XCB
Display *g_display{nullptr};
#endif

xcb_connection_t *g_connection{nullptr};
int32_t g_screenNumber{0};
xcb_screen_t *g_screen{nullptr};
xcb_ewmh_connection_t g_EWMH{};
xcb_atom_t WM_CHANGE_STATE{XCB_NONE};
xcb_atom_t WM_DELETE_WINDOW{XCB_NONE};
xcb_atom_t _NET_WM_WINDOW_OPACITY{XCB_NONE};
xcb_atom_t _USER_DATA{XCB_NONE};

xcb_key_symbols_t *g_keySymbols{nullptr};

xcb_window_t g_clipboard{XCB_WINDOW_NONE};
xcb_atom_t CLIPBOARD{XCB_NONE};
xcb_atom_t TARGETS{XCB_NONE};

const char *const g_knownTargetNames[]{
  // clang-format off
  "UTF8_STRING",
  "STRING",
  "TEXT",
  "text/plain;charset=utf-8",
  "text/plain",
  // clang-format on
};
enum KnownTarget_ {
  KnownTarget_UTF8_STRING = 0,
  KnownTarget_STRING,
  KnownTarget_TEXT,
  KnownTarget_TextPlainUTF8,
  KnownTarget_TextPlain,

  KnownTarget_Count,
};
xcb_atom_t g_knownTargets[KnownTarget_Count];

std::string g_clipboardBuffer;

bool g_cursorVisible{true};

#define _XCB_GET_COOKIE(name, ...) xcb_##name(g_connection, __VA_ARGS__)
#define _XCB_GET_REPLY(name, ...)                                              \
  xcb_##name##_reply(g_connection, _XCB_GET_COOKIE(name, __VA_ARGS__), nullptr)

#define _XCB_REPLY_T(name) xcb_##name##_reply_t
#define XCB_GET_SMARTPTR_REPLY(name, ...)                                      \
  std::unique_ptr<_XCB_REPLY_T(name), decltype(&free)> {                       \
    _XCB_GET_REPLY(name, __VA_ARGS__), free                                    \
  }

template <std::integral T>
[[nodiscard]] auto makeSpan(const xcb_get_property_reply_t *reply) {
  const auto count = xcb_get_property_value_length(reply) / sizeof(T);
  const auto values = reinterpret_cast<T *>(xcb_get_property_value(reply));
  return std::span{values, count};
}

void initClipboard() {
  g_clipboard = xcb_generate_id(g_connection);
  const auto cookie = xcb_create_window_checked(
    g_connection, XCB_COPY_FROM_PARENT, g_clipboard, g_screen->root, 0, 0, 1, 1,
    0, XCB_WINDOW_CLASS_INPUT_OUTPUT, g_screen->root_visual, 0, nullptr);
  check(cookie);

  using enum AtomGetterMode;
  CLIPBOARD = getAtom("CLIPBOARD", MustExists);
  TARGETS = getAtom("TARGETS", MustExists);

  static_assert(KnownTarget_Count == std::size(g_knownTargetNames));
  for (auto i = 0; i < KnownTarget_Count; ++i) {
    g_knownTargets[i] = getAtom(g_knownTargetNames[i], MustExists);
  }
  flush();
}
void shutdownClipboard() {
  const auto cookie = xcb_destroy_window(g_connection, g_clipboard);
  check(cookie);
}

[[nodiscard]] bool isKnownTarget(const xcb_atom_t atom) {
  const auto targets = std::span{g_knownTargets, KnownTarget_Count};
  return std::ranges::find(targets, atom) != targets.cend();
}

bool handleSelectionNotify(const xcb_selection_notify_event_t *evt) {
  assert(XCB_EVENT_RESPONSE_TYPE(evt) == XCB_SELECTION_NOTIFY);
  if (evt->property != CLIPBOARD) return false;

  const auto reply =
    XCB_GET_SMARTPTR_REPLY(get_property, true, g_clipboard, CLIPBOARD,
                           XCB_GET_PROPERTY_TYPE_ANY, 0, ~0);
  auto characters = makeSpan<char>(reply.get());
  g_clipboardBuffer.assign(characters.data(), characters.size());
  return true;
}

[[nodiscard]] SmartGenericEvent waitForEvent() {
  return {xcb_wait_for_event(g_connection), free};
}

} // namespace

void init() {
#if _X11_XCB
  assert(g_display == nullptr);
  g_display = XOpenDisplay(nullptr);
  assert(g_display);
  g_connection = XGetXCBConnection(g_display);
  g_screenNumber = XDefaultScreen(g_display);
#else
  assert(g_connection == nullptr);
  g_connection = xcb_connect(nullptr, &g_screenNumber);
#endif
  assert(g_connection && xcb_connection_has_error(g_connection) == 0);

  g_screen = xcb_aux_get_screen(g_connection, g_screenNumber);
  assert(g_screen != nullptr);

  const auto cookies = xcb_ewmh_init_atoms(g_connection, &g_EWMH);
  const auto result = xcb_ewmh_init_atoms_replies(&g_EWMH, cookies, nullptr);
  assert(result);

  using enum AtomGetterMode;
  WM_CHANGE_STATE = getAtom("WM_CHANGE_STATE", MustExists);
  WM_DELETE_WINDOW = getAtom("WM_DELETE_WINDOW", MustExists);
  _NET_WM_WINDOW_OPACITY = getAtom("_NET_WM_WINDOW_OPACITY", MustExists);
  _USER_DATA = getAtom("_USER_DATA", CreateIfNotExists);

  g_keySymbols = xcb_key_symbols_alloc(g_connection);
  assert(g_keySymbols);

  // xrandr --version
  const auto randr = XCB_GET_SMARTPTR_REPLY(
    randr_query_version, XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION);

  // Required to show/hide the mouse cursor.
  const auto xfixes = XCB_GET_SMARTPTR_REPLY(
    xfixes_query_version, XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);

  initClipboard();
}
void shutdown() {
#if _X11_XCB
  assert(g_display != nullptr);
#else
  assert(g_connection != nullptr);
#endif

  flush();

  shutdownClipboard();

  xcb_key_symbols_free(g_keySymbols);
  g_keySymbols = nullptr;
  xcb_ewmh_connection_wipe(&g_EWMH);
#if _X11_XCB
  XCloseDisplay(g_display);
  g_display = nullptr;
#else
  xcb_disconnect(g_connection);
#endif
  g_connection = nullptr;
  g_screen = nullptr;
}

xcb_connection_t *getConnection() { return g_connection; }

void check(const xcb_void_cookie_t cookie) {
  const auto *err = xcb_request_check(g_connection, cookie);
  assert(!err);
}
void flush() {
  if (xcb_flush(g_connection) == 0) {
    assert(false);
  }
}

xcb_atom_t getAtom(const std::string_view name, const AtomGetterMode mode) {
  const auto reply = XCB_GET_SMARTPTR_REPLY(
    intern_atom, std::to_underlying(mode), name.length(), name.data());
  assert(reply && reply->atom != XCB_NONE);
  return reply->atom;
}
std::string getAtomName(const xcb_atom_t atom) {
  const auto reply = XCB_GET_SMARTPTR_REPLY(get_atom_name, atom);
  const auto *buffer = xcb_get_atom_name_name(reply.get());
  const std::size_t length = xcb_get_atom_name_name_length(reply.get());
  return std::string{buffer, length};
}

void iterateMonitors(
  const std::function<void(const xcb_randr_monitor_info_t &)> &f) {
  const auto reply =
    XCB_GET_SMARTPTR_REPLY(randr_get_monitors, g_screen->root, 1);
  for (auto it = xcb_randr_get_monitors_monitors_iterator(reply.get()); it.rem;
       xcb_randr_monitor_info_next(&it)) {
    f(*it.data);
  }
}

xcb_window_t createWindow(const xcb_window_t parent, const glm::ivec2 position,
                          const glm::ivec2 extent, const float alpha,
                          const std::string_view caption) {
  // An issue with XCB_EVENT_MASK_RESIZE_REDIRECT:
  // If set, a window geometry won't be updated by WM!
  const xcb_create_window_value_list_t settings{
    .background_pixel = g_screen->black_pixel,
    // clang-format off
    .event_mask =
      // Keyboard:
      XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
      // Mouse:
      XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
      XCB_EVENT_MASK_POINTER_MOTION | 
      
      XCB_EVENT_MASK_EXPOSURE |
      XCB_EVENT_MASK_VISIBILITY_CHANGE |

      XCB_EVENT_MASK_STRUCTURE_NOTIFY |

      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | 
      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
      
      XCB_EVENT_MASK_FOCUS_CHANGE |
      XCB_EVENT_MASK_PROPERTY_CHANGE,
    // clang-format on
  };
  const xcb_window_t window = xcb_generate_id(g_connection);
  // clang-format off
  auto cookie = xcb_create_window_aux_checked(
    g_connection,
    g_screen->root_depth,
    window,
    g_screen->root, // Parent.
    0, 0, extent.x, extent.y,
    XCB_COPY_FROM_PARENT, // Border width.
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    g_screen->root_visual,
    XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
    &settings
  );
  // clang-format on
  check(cookie);

  cookie = xcb_icccm_set_wm_protocols_checked(
    g_connection, window, g_EWMH.WM_PROTOCOLS, 1, &WM_DELETE_WINDOW);
  check(cookie);

  cookie = xcb_ewmh_set_wm_window_type(&g_EWMH, window, 1,
                                       parent == XCB_WINDOW_NONE
                                         ? &g_EWMH._NET_WM_WINDOW_TYPE_NORMAL
                                         : &g_EWMH._NET_WM_WINDOW_TYPE_DOCK);
  check(cookie);
  flush();

  setCaption(window, caption);
  show(window);
  // To change a window position, the window must be mapped.
  setPosition(window, position);
  // FIXME: Setting the opacity at window creation doesn't work.
  setAlpha(window, alpha);
  hide(window);

  flush();
  return window;
}
void destroyWindow(const xcb_window_t window) {
  const auto cookie = xcb_destroy_window_checked(g_connection, window);
  check(cookie);
}

constexpr auto kUserDataLength = sizeof(uintptr_t) / sizeof(uint32_t);
static_assert(kUserDataLength > 0);

void setUserData(const xcb_window_t id, const uintptr_t userData) {
  const auto cookie = xcb_change_property_checked(
    g_connection, XCB_PROP_MODE_REPLACE, id, _USER_DATA, XCB_ATOM_CARDINAL, 32,
    kUserDataLength, &userData);
  check(cookie);
}
uintptr_t getUserData(const xcb_window_t window) {
  const auto reply =
    XCB_GET_SMARTPTR_REPLY(get_property, false, window, _USER_DATA,
                           XCB_ATOM_CARDINAL, 0, kUserDataLength);
  if (reply) {
    const auto *values =
      static_cast<uintptr_t *>(xcb_get_property_value(reply.get()));
    return *values;
  }
  return 0;
}

xcb_window_t getActiveWindow() {
  const auto cookie = xcb_ewmh_get_active_window(&g_EWMH, g_screenNumber);
  xcb_window_t window{XCB_WINDOW_NONE};
  const auto result =
    xcb_ewmh_get_active_window_reply(&g_EWMH, cookie, &window, nullptr);
  assert(result);
  return window;
}
WindowStates getWindowStates(const xcb_window_t window) {
  // _NET_WM_STATE: ATOM[]
  constexpr auto kNumPossibleAtoms = 13;

  const auto reply =
    XCB_GET_SMARTPTR_REPLY(get_property, false, window, g_EWMH._NET_WM_STATE,
                           XCB_ATOM_ATOM, 0, kNumPossibleAtoms);

  WindowStates states;
  if (reply && reply->length > 0) {
    std::ranges::copy(makeSpan<xcb_atom_t>(reply.get()),
                      std::inserter(states, states.begin()));
  }
  return states;
}
bool isMinimized(const WindowStates &states) {
  return states.contains(g_EWMH._NET_WM_STATE_HIDDEN);
}
bool isMaximized(const WindowStates &states) {
  return states.contains(g_EWMH._NET_WM_STATE_MAXIMIZED_VERT) &&
         states.contains(g_EWMH._NET_WM_STATE_MAXIMIZED_HORZ);
}

glm::ivec2 getFrameExtents(const xcb_window_t window) {
  // _NET_FRAME_EXTENTS: CARDINAL[4]/32
  //  left, right, top, bottom
  constexpr auto kNumValues = 4;
  const auto reply = XCB_GET_SMARTPTR_REPLY(get_property, false, window,
                                            g_EWMH._NET_FRAME_EXTENTS,
                                            XCB_ATOM_CARDINAL, 0, kNumValues);
  assert(reply);
  const auto values = makeSpan<uint32_t>(reply.get());
  return {values[0], values[2]};
}
glm::ivec2 getPosition(const xcb_window_t window) {
  const auto reply =
    XCB_GET_SMARTPTR_REPLY(translate_coordinates, window, g_screen->root, 0, 0);
  return {reply->dst_x, reply->dst_y};
}
glm::ivec2 getExtent(const xcb_window_t window) {
  const auto reply = XCB_GET_SMARTPTR_REPLY(get_geometry, window);
  return {reply->width, reply->height};
}

void setPosition(const xcb_window_t window, const glm::ivec2 v) {
  const uint32_t values[]{
    static_cast<uint32_t>(v.x),
    static_cast<uint32_t>(v.y),
  };
  const auto cookie = xcb_configure_window_checked(
    g_connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
  check(cookie);
}
void setExtent(const xcb_window_t window, const glm::ivec2 v) {
  const uint32_t values[]{
    static_cast<uint32_t>(v.x),
    static_cast<uint32_t>(v.y),
  };
  const auto cookie = xcb_configure_window_checked(
    g_connection, window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
    values);
  check(cookie);
}
void setAlpha(const xcb_window_t window, const float a) {
  constexpr auto kMaxValue = 0xFF'FF'FF'FFul;
  const auto opacity =
    std::clamp(static_cast<unsigned long>(kMaxValue * a), 0ul, kMaxValue);
  const auto cookie = xcb_change_property_checked(
    g_connection, XCB_PROP_MODE_REPLACE, window, _NET_WM_WINDOW_OPACITY,
    XCB_ATOM_CARDINAL, 32, 1, &opacity);
  check(cookie);
}
void setCaption(const xcb_window_t window, const std::string_view caption) {
  const auto cookie = xcb_icccm_set_wm_name_checked(
    g_connection, window, XCB_ATOM_STRING, 8, caption.length(), caption.data());
  check(cookie);
}

void show(const xcb_window_t window) {
  const auto cookie = xcb_map_window_checked(g_connection, window);
  check(cookie);
}
void hide(const xcb_window_t window) {
  const auto cookie = xcb_unmap_window_checked(g_connection, window);
  check(cookie);
}

void minimize(const xcb_window_t window) {
  xcb_client_message_event_t event{
    .response_type = XCB_CLIENT_MESSAGE,
    .format = 32,
    .sequence = 0,
    .window = window,
    .type = WM_CHANGE_STATE,
    .data = {.data32 = {XCB_ICCCM_WM_STATE_ICONIC, 0, 0, 0, 0}},
  };
  const auto cookie = xcb_send_event_checked(
    g_connection, false, g_screen->root,
    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
    reinterpret_cast<const char *>(&event));
  check(cookie);
}
void maximize(const xcb_window_t window) {
#if 1
  const auto cookie = xcb_ewmh_request_change_wm_state(
    &g_EWMH, g_screenNumber, window, XCB_EWMH_WM_STATE_ADD,
    g_EWMH._NET_WM_STATE_MAXIMIZED_HORZ, g_EWMH._NET_WM_STATE_MAXIMIZED_VERT,
    XCB_EWMH_CLIENT_SOURCE_TYPE_NORMAL);
#else
  xcb_client_message_event_t event{
    .response_type = XCB_CLIENT_MESSAGE,
    .format = 32,
    .sequence = 0,
    .window = window,
    .type = g_EWMH._NET_WM_STATE,
    .data =
      {
        .data32 =
          {
            1,
            g_EWMH._NET_WM_STATE_MAXIMIZED_HORZ,
            g_EWMH._NET_WM_STATE_MAXIMIZED_VERT,
            0,
            0,
          },
      },
  };
  const auto cookie = xcb_send_event_checked(
    g_connection, false, g_screen->root,
    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
    reinterpret_cast<const char *>(&event));
#endif
  check(cookie);
}

void focus(const xcb_window_t window) {
  const auto cookie = xcb_ewmh_request_change_active_window(
    &g_EWMH, g_screenNumber, window, XCB_EWMH_CLIENT_SOURCE_TYPE_NORMAL,
    XCB_CURRENT_TIME, getActiveWindow());
  check(cookie);
}

void close(const xcb_window_t window) {
#if 1
  const auto cookie = xcb_ewmh_request_close_window(
    &g_EWMH, g_screenNumber, window, XCB_TIME_CURRENT_TIME,
    XCB_EWMH_CLIENT_SOURCE_TYPE_NORMAL);
#else
  const xcb_client_message_event_t event{
    .response_type = XCB_CLIENT_MESSAGE,
    .format = 32,
    .sequence = 0,
    .window = window,
    .type = g_EWMH.WM_PROTOCOLS,
    .data = {
      .data32 = {WM_DELETE_WINDOW, XCB_CURRENT_TIME, 0, 0, 0},
    }};
  const auto cookie =
    xcb_send_event_checked(g_connection, false, window, XCB_EVENT_MASK_NO_EVENT,
                           reinterpret_cast<const char *>(&event));
#endif
  check(cookie);
}

SmartGenericEvent pollForEvent() {
  return {xcb_poll_for_event(g_connection), free};
}

PropertyID getChangedPropertyID(const xcb_property_notify_event_t *evt) {
  if (evt->atom == g_EWMH._NET_WM_STATE) {
    return PropertyID::State;
  } else if (evt->atom == g_EWMH._NET_FRAME_EXTENTS) {
    return PropertyID::FrameExtents;
  }
  return PropertyID::Unknown;
}
bool isCloseMessage(const xcb_client_message_event_t *evt) {
  return evt->data.data32[0] == WM_DELETE_WINDOW;
}
bool isMouseWheelEvent(const xcb_button_press_event_t *evt) {
  return (evt->detail >= 4 && evt->detail <= 7);
}

glm::ivec2 getCursorPosition() {
  const auto reply = XCB_GET_SMARTPTR_REPLY(query_pointer, g_screen->root);
  assert(reply);
  return {reply->root_x, reply->root_y};
}
void setCursorPosition(const glm::ivec2 v) {
  const auto cookie = xcb_warp_pointer_checked(
    g_connection, XCB_NONE, g_screen->root, 0, 0, 0, 0, v.x, v.y);
  check(cookie);
}

bool isCursorVisible() { return g_cursorVisible; }
void showCursor() {
  if (!g_cursorVisible) {
    const auto cookie =
      xcb_xfixes_show_cursor_checked(g_connection, g_screen->root);
    check(cookie);
    g_cursorVisible = true;
  }
}
void hideCursor() {
  if (g_cursorVisible) {
    const auto cookie =
      xcb_xfixes_hide_cursor_checked(g_connection, g_screen->root);
    check(cookie);
    g_cursorVisible = false;
  }
}

xcb_keysym_t getKeySym(const xcb_keycode_t kc) {
  return xcb_key_symbols_get_keysym(g_keySymbols, kc, 0);
}
std::optional<char> characterLookup(const xcb_keycode_t keycode,
                                    const uint16_t state) {
#if _X11_XCB
  // https://stackoverflow.com/a/47730528
  XKeyEvent e{
    .display = g_display,
    .state = state,
    .keycode = keycode,
  };
  char c{0};
  return XLookupString(&e, &c, 1, nullptr, nullptr) ? std::make_optional(c)
                                                    : std::nullopt;
#else
  return std::nullopt;
#endif
}

void setClipboardText(const std::string_view text) {
  if (g_clipboardBuffer == text) return;

  auto cookie = xcb_set_selection_owner(g_connection, g_clipboard, CLIPBOARD,
                                        XCB_CURRENT_TIME);
  check(cookie);
  cookie = xcb_xfixes_select_selection_input(
    g_connection, g_clipboard, CLIPBOARD,
    XCB_XFIXES_SELECTION_EVENT_MASK_SET_SELECTION_OWNER);
  check(cookie);
  flush();

  g_clipboardBuffer = text;
}
std::string_view getClipboardText() {
  auto reply = XCB_GET_SMARTPTR_REPLY(get_selection_owner, CLIPBOARD);
  if (!reply) return "";

  if (reply->owner != g_clipboard) {
    const auto cookie = xcb_convert_selection_checked(
      g_connection, g_clipboard, CLIPBOARD, g_knownTargets[KnownTarget_TEXT],
      CLIPBOARD, XCB_CURRENT_TIME);
    check(cookie);
    flush();
    auto evt = waitForEvent();
    handleSelectionNotify(
      reinterpret_cast<xcb_selection_notify_event_t *>(evt.get()));
  }
  return g_clipboardBuffer;
}

bool handleSelectionRequest(const xcb_selection_request_event_t *evt) {
  assert(XCB_EVENT_RESPONSE_TYPE(evt) == XCB_SELECTION_REQUEST);

  xcb_void_cookie_t cookie{};
  if (evt->target == TARGETS) {
    cookie = xcb_change_property_checked(
      g_connection, XCB_PROP_MODE_REPLACE, evt->requestor, evt->property,
      XCB_ATOM_ATOM, 32, sizeof(xcb_atom_t) * KnownTarget_Count,
      g_knownTargets);
  } else if (isKnownTarget(evt->target)) {
    cookie = xcb_change_property_checked(
      g_connection, XCB_PROP_MODE_REPLACE, evt->requestor, evt->property,
      evt->target, 8, g_clipboardBuffer.length(), g_clipboardBuffer.data());
  } else {
    return false;
  }
  check(cookie);

  const xcb_selection_notify_event_t notifyEvent{
    .response_type = XCB_SELECTION_NOTIFY,
    .time = XCB_CURRENT_TIME,
    .requestor = evt->requestor,
    .selection = evt->selection,
    .target = evt->target,
    .property = evt->property,
  };
  cookie = xcb_send_event_checked(g_connection, false, evt->requestor,
                                  XCB_EVENT_MASK_PROPERTY_CHANGE,
                                  reinterpret_cast<const char *>(&notifyEvent));
  check(cookie);
  flush();
  return true;
}

} // namespace x11
