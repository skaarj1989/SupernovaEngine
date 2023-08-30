#include "os/Window.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
#ifdef _DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif

  try {
    auto window = os::Window::Builder{}
                    .setCaption("Basic Window")
                    .setPosition({0, 0})
                    .setSize({640, 480})
                    .build();

    os::center(window);

    bool quit{false};
    window.on<os::CloseWindowEvent>([&quit](auto, auto &) { quit = true; });
    window.on<os::KeyboardEvent>([](const os::KeyboardEvent &evt,
                                    auto &sender) {
      if (evt.keyCode == os::KeyCode::Esc && evt.state == os::KeyState::Down) {
        sender.close();
      }
    });

    window.show();
    while (true) {
      os::pollEvents(&window);
      if (quit) break;
    }
  } catch (const std::exception &e) {
    std::cout << e.what();
    return -1;
  }

  return 0;
}
