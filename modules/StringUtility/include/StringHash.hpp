#pragma once

#include <string>

struct StringHash {
  using is_transparent = void;

  [[nodiscard]] std::size_t operator()(const char *str) const {
    return std::hash<std::string_view>{}(str);
  }
  [[nodiscard]] std::size_t operator()(std::string_view str) const {
    return std::hash<std::string_view>{}(str);
  }
  [[nodiscard]] std::size_t operator()(const std::string &str) const {
    return std::hash<std::string>{}(str);
  }
};
