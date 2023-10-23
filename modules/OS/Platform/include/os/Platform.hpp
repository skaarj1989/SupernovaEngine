#pragma once

#include <string>

namespace os {

class Platform {
public:
  Platform();
  ~Platform();

  static void setClipboardText(const std::string_view);
  [[nodiscard]] static std::string_view getClipboardText();
};

} // namespace os
