#include "StringUtility.hpp"
#include <array>
#include <format>

void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), ::isspace));
}
void rtrim(std::string &s) {
  s.erase(std::find_if_not(s.rbegin(), s.rend(), ::isspace).base(), s.end());
}
void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

bool contains(const std::string_view s, const std::string_view pattern) {
  constexpr auto kCaseInsensitiveMatch = [](auto ch1, auto ch2) {
    return std::toupper(ch1) == std::toupper(ch2);
  };
  return !std::ranges::search(s, pattern, kCaseInsensitiveMatch).empty();
}

std::string formatBytes(std::size_t bytes) {
  static constexpr std::array kUnits{"B", "KiB", "MiB", "GiB"};

  std::size_t i{0};
  auto size = static_cast<double>(bytes);

  // https://gist.github.com/dgoguerra/7194777
  if (bytes >= 1024) {
    for (; (bytes / 1024) > 0 && (i < kUnits.size() - 1); i++, bytes /= 1024) {
      size = bytes / 1024.0;
    }
  }
  return std::format("{:.2f} {}", size, kUnits[i]);
}
