#include "os/Window.hpp"

namespace os {

using Builder = Window::Builder;

Builder &Builder::setParent(const Window &window) {
  m_parent = &window;
  return *this;
}
Builder &Builder::setPosition(glm::ivec2 position) {
  m_position = position;
  return *this;
}
Builder &Builder::setSize(glm::ivec2 size) {
  m_size = size;
  return *this;
}
Builder &Builder::setCaption(const std::string_view caption) {
  m_caption = caption;
  return *this;
}

Window Builder::build() const {
#ifdef _WIN32
  // clang-format off
  const RECT rect{
    m_position.x,
    m_position.y,
    m_position.x + m_size.x,
    m_position.y + m_size.y
  };
  // clang-format on

  const auto style = DWORD(m_parent ? WS_POPUP : WS_OVERLAPPEDWINDOW);
  const auto exStyle = WS_EX_APPWINDOW | WS_EX_LAYERED;

  return Window{m_parent ? m_parent->m_native.hWnd : HWND_DESKTOP, rect, style,
                exStyle, m_caption};
#elif defined __linux
  // TODO ...
#else
  return {};
#endif
}

} // namespace os
