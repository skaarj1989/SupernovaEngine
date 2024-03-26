#include "RmlUiPlatformInterface.hpp"
#include "VisitorHelper.hpp"
#include "os/Platform.hpp"
#include "os/FileSystem.hpp"
#include "RmlUi/Core/Context.h"

//
// RmlUiSystemInterface class:
//

RmlUiSystemInterface::RmlUiSystemInterface() { m_start = clock::now(); }

double RmlUiSystemInterface::GetElapsedTime() {
  using dsec = std::chrono::duration<double>;
  return dsec{clock::now() - m_start}.count();
}

void RmlUiSystemInterface::SetClipboardText(const Rml::String &in) {
  os::Platform::setClipboardText(in);
}
void RmlUiSystemInterface::GetClipboardText(Rml::String &out) {
  out = os::Platform::getClipboardText();
}

//
// RmlUiFileInterface class:
//

Rml::FileHandle RmlUiFileInterface::Open(const Rml::String &path) {
  auto f = os::FileSystem::mapFile(os::FileSystem::getRoot() / path);
  return Rml::FileHandle(f.release());
}
void RmlUiFileInterface::Close(Rml::FileHandle file) {
  auto *f = std::bit_cast<os::DataStream *>(file);
  f->close();
  delete f;
}
size_t RmlUiFileInterface::Read(void *buffer, size_t size,
                                Rml::FileHandle file) {
  return std::bit_cast<os::DataStream *>(file)->read(buffer, size);
}
bool RmlUiFileInterface::Seek(Rml::FileHandle file, long offset, int origin) {
  return std::bit_cast<os::DataStream *>(file)->seek(
           offset, static_cast<os::DataStream::Origin>(origin)) == 0;
}
size_t RmlUiFileInterface::Tell(Rml::FileHandle file) {
  return std::bit_cast<os::DataStream *>(file)->tell();
}

namespace {

[[nodiscard]] std::optional<Rml::Input::KeyIdentifier>
remapKeyCode(const os::KeyCode keyCode) {
  using namespace Rml::Input;

  switch (keyCode) {
  case os::KeyCode::Tab:
    return KI_TAB;
  case os::KeyCode::Left:
    return KI_LEFT;
  case os::KeyCode::Right:
    return KI_RIGHT;
  case os::KeyCode::Up:
    return KI_UP;
  case os::KeyCode::Down:
    return KI_DOWN;
  case os::KeyCode::Prior:
    return KI_PRIOR;
  case os::KeyCode::Next:
    return KI_NEXT;
  case os::KeyCode::Home:
    return KI_HOME;
  case os::KeyCode::End:
    return KI_END;
  case os::KeyCode::Insert:
    return KI_INSERT;
  case os::KeyCode::Delete:
    return KI_DELETE;
  case os::KeyCode::Backspace:
    return KI_BACK;
  case os::KeyCode::Space:
    return KI_SPACE;
  case os::KeyCode::Return:
    return KI_RETURN;
  case os::KeyCode::Esc:
    return KI_ESCAPE;
  case os::KeyCode::Captial:
    return KI_CAPITAL;
  case os::KeyCode::SLock:
    return KI_SCROLL;
  case os::KeyCode::NumLock:
    return KI_NUMLOCK;

  case os::KeyCode::Snapshot:
    return KI_SNAPSHOT;
  case os::KeyCode::Pause:
    return KI_PAUSE;
  case os::KeyCode::Num0:
    return KI_NUMPAD0;
  case os::KeyCode::Num1:
    return KI_NUMPAD1;
  case os::KeyCode::Num2:
    return KI_NUMPAD2;
  case os::KeyCode::Num3:
    return KI_NUMPAD3;
  case os::KeyCode::Num4:
    return KI_NUMPAD4;
  case os::KeyCode::Num5:
    return KI_NUMPAD5;
  case os::KeyCode::Num6:
    return KI_NUMPAD6;
  case os::KeyCode::Num7:
    return KI_NUMPAD7;
  case os::KeyCode::Num8:
    return KI_NUMPAD8;
  case os::KeyCode::Num9:
    return KI_NUMPAD9;

  case os::KeyCode::Multiply:
    return KI_MULTIPLY;
  case os::KeyCode::Add:
    return KI_ADD;

  case os::KeyCode::Subtract:
    return KI_SUBTRACT;
  case os::KeyCode::Decimal:
    return KI_DECIMAL;
  case os::KeyCode::Divide:
    return KI_DIVIDE;

  case os::KeyCode::LShift:
    return KI_LSHIFT;
  case os::KeyCode::RShift:
    return KI_RSHIFT;
  case os::KeyCode::LControl:
    return KI_LCONTROL;
  case os::KeyCode::RControl:
    return KI_RCONTROL;
  case os::KeyCode::LMenu:
    return KI_LMENU;
  case os::KeyCode::RMenu:
    return KI_RMENU;
  case os::KeyCode::LWin:
    return KI_LWIN;
  case os::KeyCode::RWin:
    return KI_RWIN;

  case os::KeyCode::Apps:
    return KI_APPS;

  case os::KeyCode::_0:
    return KI_0;
  case os::KeyCode::_1:
    return KI_1;
  case os::KeyCode::_2:
    return KI_2;
  case os::KeyCode::_3:
    return KI_3;
  case os::KeyCode::_4:
    return KI_4;
  case os::KeyCode::_5:
    return KI_5;
  case os::KeyCode::_6:
    return KI_6;
  case os::KeyCode::_7:
    return KI_7;
  case os::KeyCode::_8:
    return KI_8;
  case os::KeyCode::_9:
    return KI_9;

  case os::KeyCode::A:
    return KI_A;
  case os::KeyCode::B:
    return KI_B;
  case os::KeyCode::C:
    return KI_C;
  case os::KeyCode::D:
    return KI_D;
  case os::KeyCode::E:
    return KI_E;
  case os::KeyCode::F:
    return KI_F;
  case os::KeyCode::G:
    return KI_G;
  case os::KeyCode::H:
    return KI_H;
  case os::KeyCode::I:
    return KI_I;
  case os::KeyCode::J:
    return KI_J;
  case os::KeyCode::K:
    return KI_K;
  case os::KeyCode::L:
    return KI_L;
  case os::KeyCode::M:
    return KI_M;
  case os::KeyCode::N:
    return KI_N;
  case os::KeyCode::O:
    return KI_O;
  case os::KeyCode::P:
    return KI_P;
  case os::KeyCode::Q:
    return KI_Q;
  case os::KeyCode::R:
    return KI_R;
  case os::KeyCode::S:
    return KI_S;
  case os::KeyCode::T:
    return KI_T;
  case os::KeyCode::U:
    return KI_U;
  case os::KeyCode::V:
    return KI_V;
  case os::KeyCode::W:
    return KI_W;
  case os::KeyCode::X:
    return KI_X;
  case os::KeyCode::Y:
    return KI_Y;
  case os::KeyCode::Z:
    return KI_Z;

  case os::KeyCode::F1:
    return KI_F1;
  case os::KeyCode::F2:
    return KI_F2;
  case os::KeyCode::F3:
    return KI_F3;
  case os::KeyCode::F4:
    return KI_F4;
  case os::KeyCode::F5:
    return KI_F5;
  case os::KeyCode::F6:
    return KI_F6;
  case os::KeyCode::F7:
    return KI_F7;
  case os::KeyCode::F8:
    return KI_F8;
  case os::KeyCode::F9:
    return KI_F9;
  case os::KeyCode::F10:
    return KI_F10;
  case os::KeyCode::F11:
    return KI_F11;
  case os::KeyCode::F12:
    return KI_F12;
  }
  return std::nullopt;
}

} // namespace

void processEvent(Rml::Context &ctx, const os::InputEvent &evt) {
  std::visit(Overload{
               [&ctx](const os::MouseMoveEvent &evt_) {
                 ctx.ProcessMouseMove(evt_.position.x, evt_.position.y, 0);
               },
               [&ctx](const os::MouseButtonEvent &evt_) {
                 const auto btn = static_cast<int32_t>(evt_.button);

                 switch (evt_.state) {
                   using enum os::MouseButtonState;

                 case Pressed:
                   ctx.ProcessMouseButtonDown(btn, 0);
                   break;

                 case Released:
                   ctx.ProcessMouseButtonUp(btn, 0);
                   break;
                 }
               },
               [&ctx](const os::MouseWheelEvent &evt_) {
                 Rml::Vector2f val;
                 *(evt_.wheel == os::MouseWheel::Horizontal ? &val.x
                                                            : &val.y) +=
                   -evt_.step;
                 ctx.ProcessMouseWheel(val, 0);
               },
               [&ctx](const os::KeyboardEvent &evt_) {
                 const auto ki =
                   remapKeyCode(evt_.keyCode).value_or(Rml::Input::KI_UNKNOWN);

                 switch (evt_.state) {
                   using enum os::KeyState;

                 case Down:
                   ctx.ProcessKeyDown(ki, 0);
                   break;
                 case Up:
                   ctx.ProcessKeyUp(ki, 0);
                   break;
                 }
               },
               [&ctx](const os::InputCharacterEvent &evt_) {
                 ctx.ProcessTextInput(evt_.c);
               },
             },
             evt);
}
