#include "LuaWindow.hpp"
#include "sol/state.hpp"

#include "os/Window.hpp"
#include "Sol2HelperMacros.hpp"

using namespace os;

namespace {

void registerMouseEvents(sol::state &lua) {
  // clang-format off
  DEFINE_USERTYPE(MouseMoveEvent,
    sol::no_constructor,
    _BIND(MouseMoveEvent, position),

    BIND_TYPEID(MouseMoveEvent),
    BIND_TOSTRING(MouseMoveEvent)
  );
  // clang-format on
#define MAKE_PAIR(Key) _MAKE_PAIR(MouseButton, Key)
  DEFINE_ENUM(MouseButton, {
                             MAKE_PAIR(Undefined),
                             MAKE_PAIR(Left),
                             MAKE_PAIR(Right),
                             MAKE_PAIR(Middle),
                             MAKE_PAIR(X1),
                             MAKE_PAIR(X2),
                           });
#undef MAKE_PAIR

#define MAKE_PAIR(Key) _MAKE_PAIR(MouseButtonState, Key)
  DEFINE_ENUM(MouseButtonState, {
                                  MAKE_PAIR(Undefined),
                                  MAKE_PAIR(Released),
                                  MAKE_PAIR(Pressed),
                                  MAKE_PAIR(DblClick),
                                });
#undef MAKE_PAIR

#define BIND(Member) _BIND(MouseButtonEvent, Member)
  // clang-format off
  DEFINE_USERTYPE(MouseButtonEvent,
    sol::no_constructor,

    BIND(position),
    BIND(state),
    BIND(button),

    BIND_TYPEID(MouseButtonEvent),
    BIND_TOSTRING(MouseButtonEvent)
  );
  // clang-format on
#undef BIND

#define MAKE_PAIR(Key) _MAKE_PAIR(MouseWheel, Key)
  DEFINE_ENUM(MouseWheel, {
                            MAKE_PAIR(Undefined),
                            MAKE_PAIR(Horizontal),
                            MAKE_PAIR(Vertical),
                          });
#undef MAKE_PAIR

#define BIND(Member) _BIND(MouseWheelEvent, Member)
  // clang-format off
  DEFINE_USERTYPE(MouseWheelEvent,
    sol::no_constructor,

    BIND(wheel),
    BIND(step),

    BIND_TYPEID(MouseWheelEvent),
    BIND_TOSTRING(MouseWheelEvent)
  );
  // clang-format on
#undef BIND
}
void registerKeyEvents(sol::state &lua) {
  // -- KeyState enum:

#define MAKE_PAIR(Key) _MAKE_PAIR(KeyState, Key)
  DEFINE_ENUM(KeyState, {
                          MAKE_PAIR(Undefined),
                          MAKE_PAIR(Up),
                          MAKE_PAIR(Down),
                        });
#undef MAKE_PAIR

  // -- KeyCode enum:

#define MAKE_PAIR(Key) _MAKE_PAIR(KeyCode, Key)
  // clang-format off
  DEFINE_ENUM(KeyCode, {
                         MAKE_PAIR(Backspace),
                         MAKE_PAIR(Tab),
                         MAKE_PAIR(Clear),
                         MAKE_PAIR(Return),
                         MAKE_PAIR(Shift),
                         MAKE_PAIR(Control),
                         MAKE_PAIR(Menu),
                         MAKE_PAIR(Pause),
                         MAKE_PAIR(Captial),
                         MAKE_PAIR(Esc),
                         MAKE_PAIR(Space),

                         MAKE_PAIR(Prior),
                         MAKE_PAIR(Next),
                         MAKE_PAIR(End),
                         MAKE_PAIR(Home),

                         MAKE_PAIR(Left),
                         MAKE_PAIR(Up),
                         MAKE_PAIR(Right),
                         MAKE_PAIR(Down),

                         MAKE_PAIR(Snapshot),
                         MAKE_PAIR(Insert),
                         MAKE_PAIR(Delete),

                         MAKE_PAIR(LWin),
                         MAKE_PAIR(RWin),
                         MAKE_PAIR(Apps),

                         MAKE_PAIR(_0),
                         MAKE_PAIR(_1),
                         MAKE_PAIR(_2),
                         MAKE_PAIR(_3),
                         MAKE_PAIR(_4),
                         MAKE_PAIR(_5),
                         MAKE_PAIR(_6),
                         MAKE_PAIR(_7),
                         MAKE_PAIR(_8),
                         MAKE_PAIR(_9),

                         MAKE_PAIR(A),
                         MAKE_PAIR(B),
                         MAKE_PAIR(C),
                         MAKE_PAIR(D),
                         MAKE_PAIR(E),
                         MAKE_PAIR(F),
                         MAKE_PAIR(G),
                         MAKE_PAIR(H),
                         MAKE_PAIR(I),
                         MAKE_PAIR(J),
                         MAKE_PAIR(K),
                         MAKE_PAIR(L),
                         MAKE_PAIR(M),
                         MAKE_PAIR(N),
                         MAKE_PAIR(O),
                         MAKE_PAIR(P),
                         MAKE_PAIR(Q),
                         MAKE_PAIR(R),
                         MAKE_PAIR(S),
                         MAKE_PAIR(T),
                         MAKE_PAIR(U),
                         MAKE_PAIR(V),
                         MAKE_PAIR(W),
                         MAKE_PAIR(Y),
                         MAKE_PAIR(Z),

                         MAKE_PAIR(Num0),
                         MAKE_PAIR(Num1),
                         MAKE_PAIR(Num2),
                         MAKE_PAIR(Num3),
                         MAKE_PAIR(Num4),
                         MAKE_PAIR(Num5),
                         MAKE_PAIR(Num7),
                         MAKE_PAIR(Num8),
                         MAKE_PAIR(Num9),
                         MAKE_PAIR(Multiply),
                         MAKE_PAIR(Add),
                         MAKE_PAIR(Separator),
                         MAKE_PAIR(Subtract),
                         MAKE_PAIR(Decimal),
                         MAKE_PAIR(Divide),

                         MAKE_PAIR(F1),
                         MAKE_PAIR(F2),
                         MAKE_PAIR(F3),
                         MAKE_PAIR(F4),
                         MAKE_PAIR(F5),
                         MAKE_PAIR(F6),
                         MAKE_PAIR(F7),
                         MAKE_PAIR(F8),
                         MAKE_PAIR(F9),
                         MAKE_PAIR(F10),
                         MAKE_PAIR(F11),
                         MAKE_PAIR(F12),

                         MAKE_PAIR(NumLock),
                         MAKE_PAIR(SLock),

                         MAKE_PAIR(LShift),
                         MAKE_PAIR(RShift),
                         MAKE_PAIR(LControl),
                         MAKE_PAIR(RControl),
                         MAKE_PAIR(LMenu),
                         MAKE_PAIR(RMenu),
                       });
  // clang-format on
#undef MAKE_PAIR

  // -- KeyboardEvent struct:

#define BIND(Member) _BIND(KeyboardEvent, Member)
  // clang-format off
  DEFINE_USERTYPE(KeyboardEvent,
    sol::no_constructor,
    
    BIND(state),
    BIND(keyCode),
    BIND(charCode),
    BIND(repeated),

    BIND_TYPEID(KeyboardEvent),
    BIND_TOSTRING(KeyboardEvent)
  );
#undef BIND

  DEFINE_USERTYPE(InputCharacterEvent,
    sol::no_constructor,
    
    _BIND(InputCharacterEvent, c),

    BIND_TYPEID(InputCharacterEvent),
    BIND_TOSTRING(InputCharacterEvent)
  );
  // clang-format on
}

void registerFreeFunctions(sol::state &lua) {
  lua["getAspectRatio"] = getAspectRatio;
  lua["center"] = center;
  lua["getCenter"] = getCenter;
}

} // namespace

void registerWindow(sol::state &lua) {
  registerKeyEvents(lua);
  registerMouseEvents(lua);

#define BIND(Member) _BIND(Window, Member)
  // clang-format off
  DEFINE_USERTYPE(Window,
    sol::no_constructor,

    BIND(setPosition),
    BIND(setExtent),
    BIND(setAlpha),
    
    BIND(setCaption),

    // ---

    "getPosition", sol::overload(
      [](const Window &self) { return self.getPosition(); },
      sol::resolve<glm::ivec2(const Window::Area) const>(&Window::getPosition)
    ),
    "getExtent", sol::overload(
      [](const Window &self) { return self.getExtent(); },
      sol::resolve<glm::ivec2(const Window::Area) const>(&Window::getExtent)
    ),
    BIND(getCaption),

    // ---

    BIND(getState),
    BIND(hasFocus),

    // ---

    BIND(show),
    BIND(hide),

    BIND(minimize),
    BIND(maximize),

    BIND(focus),
    
    BIND(close),

    BIND_TOSTRING(Window)
  );

#define MAKE_PAIR(Value) _MAKE_PAIR(Window::Area, Value)
  lua["Window"].get<sol::table>().new_enum<Window::Area>("Area", {
    MAKE_PAIR(Client),
    MAKE_PAIR(Absolute),
  });
#undef MAKE_PAIR
#undef BIND
  // clang-format on

  registerFreeFunctions(lua);
}
