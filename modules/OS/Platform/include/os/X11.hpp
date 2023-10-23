#pragma once

#include <xcb/xcb.h>
#include <xcb/randr.h>

#include <memory>
#include <functional>
#include <set>
#include <string>
#include <optional>

#include "glm/ext/vector_int2.hpp"

namespace x11 {

void init();
void shutdown();

[[nodiscard]] xcb_connection_t *getConnection();

void check(const xcb_void_cookie_t);
void flush();

// ---

enum class AtomGetterMode : int8_t { CreateIfNotExists = 0, MustExists = 1 };
[[nodiscard]] xcb_atom_t getAtom(const std::string_view name,
                                 const AtomGetterMode);
[[nodiscard]] std::string getAtomName(const xcb_atom_t);

// ---

void iterateMonitors(
  const std::function<void(const xcb_randr_monitor_info_t &)> &);

[[nodiscard]] xcb_window_t createWindow(const xcb_window_t parent,
                                        const glm::ivec2 position,
                                        const glm::ivec2 extent,
                                        const float alpha,
                                        const std::string_view caption);
void destroyWindow(const xcb_window_t);

void setUserData(const xcb_window_t, const uintptr_t);
[[nodiscard]] uintptr_t getUserData(const xcb_window_t);

[[nodiscard]] xcb_window_t getActiveWindow();

using WindowStates = std::set<xcb_atom_t>;
[[nodiscard]] WindowStates getWindowStates(const xcb_window_t);
[[nodiscard]] bool isMinimized(const WindowStates &);
[[nodiscard]] bool isMaximized(const WindowStates &);

// @return The size of the title bar.
[[nodiscard]] glm::ivec2 getFrameExtents(const xcb_window_t);
// @return Top-left coord of the client/work area.
[[nodiscard]] glm::ivec2 getPosition(const xcb_window_t);
// @return The size of the client/work area.
[[nodiscard]] glm::ivec2 getExtent(const xcb_window_t);

// @param p Top-left coord (including non-client area).
void setPosition(const xcb_window_t, const glm::ivec2 p);
// @param extent Total window extent (including non-client area).
void setExtent(const xcb_window_t, const glm::ivec2 extent);
void setAlpha(const xcb_window_t, const float);
void setCaption(const xcb_window_t, const std::string_view);

void show(const xcb_window_t);
void hide(const xcb_window_t);

void minimize(const xcb_window_t);
void maximize(const xcb_window_t);

void focus(const xcb_window_t);

void close(const xcb_window_t);

using SmartGenericEvent = std::unique_ptr<xcb_generic_event_t, decltype(&free)>;
[[nodiscard]] SmartGenericEvent pollForEvent();

enum class PropertyID { Unknown, State, FrameExtents };
[[nodiscard]] PropertyID
getChangedPropertyID(const xcb_property_notify_event_t *);
[[nodiscard]] bool isCloseMessage(const xcb_client_message_event_t *);
[[nodiscard]] bool isMouseWheelEvent(const xcb_button_press_event_t *);

// ---

[[nodiscard]] glm::ivec2 getCursorPosition();
void setCursorPosition(const glm::ivec2);

[[nodiscard]] bool isCursorVisible();
void showCursor();
void hideCursor();

// ---

[[nodiscard]] xcb_keysym_t getKeySym(const xcb_keycode_t);
[[nodiscard]] std::optional<char> characterLookup(const xcb_keycode_t keycode,
                                                  const uint16_t state);

// ---

void setClipboardText(const std::string_view);
[[nodiscard]] std::string_view getClipboardText();

bool handleSelectionRequest(const xcb_selection_request_event_t *);

} // namespace x11
