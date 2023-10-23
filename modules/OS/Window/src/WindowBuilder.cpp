#include "os/Window.hpp"

namespace os {

using Builder = Window::Builder;

Builder &Builder::setParent(const Window &window) {
  m_parent = &window;
  return *this;
}
Builder &Builder::setPosition(const Position position) {
  m_position = position;
  return *this;
}
Builder &Builder::setExtent(const Extent extent) {
  m_extent = extent;
  return *this;
}
Builder &Window::Builder::setAlpha(const float a) {
  m_alpha = a;
  return *this;
}
Builder &Builder::setCaption(const std::string_view caption) {
  m_caption = caption;
  return *this;
}

Window Builder::build() const {
  return Window{m_parent, m_position, m_extent, m_alpha, m_caption};
}

} // namespace os
