#include "os/Platform.hpp"
#include <Windows.h>

namespace os {

Platform::Platform() = default;
Platform::~Platform() = default;

void Platform::setClipboardText(const std::string_view text) {
  if (!::OpenClipboard(HWND_DESKTOP)) return;

  if (auto hMem = ::GlobalAlloc(GMEM_MOVEABLE, text.length() + 1); hMem) {
    if (auto str = static_cast<char *>(::GlobalLock(hMem)); str) {
      std::memcpy(str, text.data(), text.length());
      str[text.length()] = 0;
      ::GlobalUnlock(hMem);
      ::EmptyClipboard();
      if (!::SetClipboardData(CF_TEXT, hMem)) {
        ::GlobalFree(hMem);
      }
    }
  }
  ::CloseClipboard();
}
std::string_view Platform::getClipboardText() {
  std::string_view text;

  if (::OpenClipboard(HWND_DESKTOP)) {
    if (auto hData = ::GetClipboardData(CF_TEXT); hData) {
      text = static_cast<char *>(::GlobalLock(hData));
      ::GlobalUnlock(hData);
    }
    ::CloseClipboard();
  }
  return text;
}

} // namespace os
