#include "ImGuiPlatformInterface.hpp"
#include "os/Monitor.hpp"
#include "imgui.h"

namespace {

[[nodiscard]] auto *getWindow(const ImGuiViewport *viewport) {
  assert(viewport);
  return static_cast<os::Window *>(viewport->PlatformHandle);
}

#ifdef IMGUI_HAS_VIEWPORT
[[nodiscard]] ImGuiPlatformMonitor convert(const os::MonitorInfo &monitor) {
  ImGuiPlatformMonitor pm{};
  pm.MainPos = glm::vec2{monitor.mainArea.position};
  pm.MainSize = glm::vec2{monitor.mainArea.size};
  pm.WorkPos = glm::vec2{monitor.workArea.position};
  pm.WorkSize = glm::vec2{monitor.workArea.size};
  pm.DpiScale = monitor.dpi;
  return pm;
};
void updateMonitors() {
  const auto src = os::getMonitors();
  auto &dst = ImGui::GetPlatformIO().Monitors;
  dst.clear();
  dst.reserve(src.size());
  std::ranges::transform(src, std::back_inserter(dst), convert);
}
#endif

} // namespace

std::optional<ImGuiKey> remapKeyCode(os::KeyCode keyCode) {
  switch (keyCode) {
  case os::KeyCode::Tab:
    return ImGuiKey_Tab;
  case os::KeyCode::Left:
    return ImGuiKey_LeftArrow;
  case os::KeyCode::Right:
    return ImGuiKey_RightArrow;
  case os::KeyCode::Up:
    return ImGuiKey_UpArrow;
  case os::KeyCode::Down:
    return ImGuiKey_DownArrow;
  case os::KeyCode::Prior:
    return ImGuiKey_PageUp;
  case os::KeyCode::Next:
    return ImGuiKey_PageDown;
  case os::KeyCode::Home:
    return ImGuiKey_Home;
  case os::KeyCode::End:
    return ImGuiKey_End;
  case os::KeyCode::Insert:
    return ImGuiKey_Insert;
  case os::KeyCode::Delete:
    return ImGuiKey_Delete;
  case os::KeyCode::Backspace:
    return ImGuiKey_Backspace;
  case os::KeyCode::Space:
    return ImGuiKey_Space;
  case os::KeyCode::Return:
    return ImGuiKey_Enter;
  case os::KeyCode::Esc:
    return ImGuiKey_Escape;
  case os::KeyCode::Captial:
    return ImGuiKey_CapsLock;
  case os::KeyCode::SLock:
    return ImGuiKey_ScrollLock;
  case os::KeyCode::NumLock:
    return ImGuiKey_NumLock;

  case os::KeyCode::Snapshot:
    return ImGuiKey_PrintScreen;
  case os::KeyCode::Pause:
    return ImGuiKey_Pause;
  case os::KeyCode::Num0:
    return ImGuiKey_Keypad0;
  case os::KeyCode::Num1:
    return ImGuiKey_Keypad1;
  case os::KeyCode::Num2:
    return ImGuiKey_Keypad2;
  case os::KeyCode::Num3:
    return ImGuiKey_Keypad3;
  case os::KeyCode::Num4:
    return ImGuiKey_Keypad4;
  case os::KeyCode::Num5:
    return ImGuiKey_Keypad5;
  case os::KeyCode::Num6:
    return ImGuiKey_Keypad6;
  case os::KeyCode::Num7:
    return ImGuiKey_Keypad7;
  case os::KeyCode::Num8:
    return ImGuiKey_Keypad8;
  case os::KeyCode::Num9:
    return ImGuiKey_Keypad9;

  case os::KeyCode::Multiply:
    return ImGuiKey_KeypadMultiply;
  case os::KeyCode::Add:
    return ImGuiKey_KeypadAdd;

  case os::KeyCode::Subtract:
    return ImGuiKey_KeypadSubtract;
  case os::KeyCode::Decimal:
    return ImGuiKey_KeypadDecimal;
  case os::KeyCode::Divide:
    return ImGuiKey_KeypadDivide;

  case os::KeyCode::LShift:
    return ImGuiKey_LeftShift;
  case os::KeyCode::RShift:
    return ImGuiKey_RightShift;
  case os::KeyCode::LControl:
    return ImGuiKey_LeftCtrl;
  case os::KeyCode::RControl:
    return ImGuiKey_RightCtrl;
  case os::KeyCode::LMenu:
    return ImGuiKey_LeftAlt;
  case os::KeyCode::RMenu:
    return ImGuiKey_RightAlt;
  case os::KeyCode::LWin:
    return ImGuiKey_LeftSuper;
  case os::KeyCode::RWin:
    return ImGuiKey_RightSuper;

  case os::KeyCode::Apps:
    return ImGuiKey_Menu;

  case os::KeyCode::_0:
    return ImGuiKey_0;
  case os::KeyCode::_1:
    return ImGuiKey_1;
  case os::KeyCode::_2:
    return ImGuiKey_2;
  case os::KeyCode::_3:
    return ImGuiKey_3;
  case os::KeyCode::_4:
    return ImGuiKey_4;
  case os::KeyCode::_5:
    return ImGuiKey_5;
  case os::KeyCode::_6:
    return ImGuiKey_6;
  case os::KeyCode::_7:
    return ImGuiKey_7;
  case os::KeyCode::_8:
    return ImGuiKey_8;
  case os::KeyCode::_9:
    return ImGuiKey_9;

  case os::KeyCode::A:
    return ImGuiKey_A;
  case os::KeyCode::B:
    return ImGuiKey_B;
  case os::KeyCode::C:
    return ImGuiKey_C;
  case os::KeyCode::D:
    return ImGuiKey_D;
  case os::KeyCode::E:
    return ImGuiKey_E;
  case os::KeyCode::F:
    return ImGuiKey_F;
  case os::KeyCode::G:
    return ImGuiKey_G;
  case os::KeyCode::H:
    return ImGuiKey_H;
  case os::KeyCode::I:
    return ImGuiKey_I;
  case os::KeyCode::J:
    return ImGuiKey_J;
  case os::KeyCode::K:
    return ImGuiKey_K;
  case os::KeyCode::L:
    return ImGuiKey_L;
  case os::KeyCode::M:
    return ImGuiKey_M;
  case os::KeyCode::N:
    return ImGuiKey_N;
  case os::KeyCode::O:
    return ImGuiKey_O;
  case os::KeyCode::P:
    return ImGuiKey_P;
  case os::KeyCode::Q:
    return ImGuiKey_Q;
  case os::KeyCode::R:
    return ImGuiKey_R;
  case os::KeyCode::S:
    return ImGuiKey_S;
  case os::KeyCode::T:
    return ImGuiKey_T;
  case os::KeyCode::U:
    return ImGuiKey_U;
  case os::KeyCode::V:
    return ImGuiKey_V;
  case os::KeyCode::W:
    return ImGuiKey_W;
  case os::KeyCode::X:
    return ImGuiKey_X;
  case os::KeyCode::Y:
    return ImGuiKey_Y;
  case os::KeyCode::Z:
    return ImGuiKey_Z;

  case os::KeyCode::F1:
    return ImGuiKey_F1;
  case os::KeyCode::F2:
    return ImGuiKey_F2;
  case os::KeyCode::F3:
    return ImGuiKey_F3;
  case os::KeyCode::F4:
    return ImGuiKey_F4;
  case os::KeyCode::F5:
    return ImGuiKey_F5;
  case os::KeyCode::F6:
    return ImGuiKey_F6;
  case os::KeyCode::F7:
    return ImGuiKey_F7;
  case os::KeyCode::F8:
    return ImGuiKey_F8;
  case os::KeyCode::F9:
    return ImGuiKey_F9;
  case os::KeyCode::F10:
    return ImGuiKey_F10;
  case os::KeyCode::F11:
    return ImGuiKey_F11;
  case os::KeyCode::F12:
    return ImGuiKey_F12;
  }
  return std::nullopt;
}
std::optional<ImGuiKey> getKeyModifier(os::KeyCode keyCode) {
  switch (keyCode) {
    using enum os::KeyCode;

  case LControl:
  case RControl:
    return ImGuiMod_Ctrl;

  case LShift:
  case RShift:
    return ImGuiMod_Shift;

  case LMenu:
  case RMenu:
    return ImGuiMod_Alt;

  case LWin:
  case RWin:
    return ImGuiMod_Super;
  }
  return std::nullopt;
}

void setupPlatformInterface(os::Window &mainWindow) {
  auto &io = ImGui::GetIO();

#ifdef WIN32
  io.BackendPlatformName = "WIN32";
#endif

  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

#ifdef IMGUI_HAS_VIEWPORT
  if (!(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) return;

  io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

  ImGui::GetMainViewport()->PlatformHandle = &mainWindow;
  updateMonitors();

  auto &platformIo = ImGui::GetPlatformIO();
  platformIo.Platform_CreateWindow = [](ImGuiViewport *viewport) {
    os::Window::Builder builder{};
    if (const auto *parentViewport =
          ImGui::FindViewportByID(viewport->ParentViewportId);
        parentViewport) {
      builder.setParent(*getWindow(parentViewport));
    }
    builder.setPosition(glm::vec2{viewport->Pos})
      .setSize(glm::vec2{viewport->Size});
    auto window = std::make_unique<os::Window>(builder.build());

    window->on<os::CloseWindowEvent>([](auto, auto &sender) {
      ImGui::FindViewportByPlatformHandle(&sender)->PlatformRequestClose = true;
    });

    window->on<os::ResizeWindowEvent>([](const auto &, auto &sender) {
      ImGui::FindViewportByPlatformHandle(&sender)->PlatformRequestResize =
        true;
    });
    window->on<os::MoveWindowEvent>([](const auto &, auto &sender) {
      ImGui::FindViewportByPlatformHandle(&sender)->PlatformRequestMove = true;
    });

    window->on<os::MouseButtonEvent>(
      [](const os::MouseButtonEvent &evt, auto &) {
        ImGui::GetIO().AddMouseButtonEvent(
          int32_t(evt.button), evt.state == os::MouseButtonState::Pressed);
      });
    window->on<os::MouseWheelEvent>([](const os::MouseWheelEvent &evt, auto &) {
      auto x = evt.step;
      decltype(x) y = 0;
      if (evt.wheel == os::MouseWheel::Vertical) std::swap(x, y);

      ImGui::GetIO().AddMouseWheelEvent(x, y);
    });

    viewport->PlatformHandle = window.release();
    viewport->PlatformRequestResize = false;
  };
  platformIo.Platform_DestroyWindow = [](ImGuiViewport *viewport) {
    if (!(viewport->Flags & ImGuiViewportFlags_OwnedByApp)) {
      if (auto *window = getWindow(viewport)) delete window;
    }
    viewport->PlatformHandle = nullptr;
  };
  platformIo.Platform_ShowWindow = [](ImGuiViewport *viewport) {
    getWindow(viewport)->show();
  };
  platformIo.Platform_SetWindowPos = [](ImGuiViewport *viewport, ImVec2 pos) {
    getWindow(viewport)->setPosition(glm::vec2{pos});
  };
  platformIo.Platform_GetWindowPos = [](ImGuiViewport *viewport) {
    return ImVec2{getWindow(viewport)->getPosition()};
  };
  platformIo.Platform_SetWindowSize = [](ImGuiViewport *viewport, ImVec2 size) {
    getWindow(viewport)->setSize(glm::vec2{size});
  };
  platformIo.Platform_GetWindowSize = [](ImGuiViewport *viewport) {
    return ImVec2{getWindow(viewport)->getClientSize()};
  };
  platformIo.Platform_SetWindowFocus = [](ImGuiViewport *viewport) {
    getWindow(viewport)->focus();
  };
  platformIo.Platform_GetWindowFocus = [](ImGuiViewport *viewport) {
    return getWindow(viewport)->hasFocus();
  };
  platformIo.Platform_GetWindowMinimized = [](ImGuiViewport *viewport) {
    return getWindow(viewport)->isMinimized();
  };
  platformIo.Platform_SetWindowTitle = [](ImGuiViewport *viewport,
                                          const char *str) {
    getWindow(viewport)->setCaption(str);
  };
  platformIo.Platform_SetWindowAlpha = [](ImGuiViewport *viewport,
                                          float alpha) {
    getWindow(viewport)->setAlpha(alpha);
  };
#endif
}
