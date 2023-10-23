#include "os/Platform.hpp"
#include "os/X11.hpp"

namespace os {

Platform::Platform() { x11::init(); }
Platform::~Platform() { x11::shutdown(); }

void Platform::setClipboardText(const std::string_view text) {
  x11::setClipboardText(text);
}
std::string_view Platform::getClipboardText() {
  return x11::getClipboardText();
}

} // namespace os
