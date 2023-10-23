#pragma once

#include <sstream>
#include <algorithm> // copy
#include <iterator>

void ltrim(std::string &);
void rtrim(std::string &);
void trim(std::string &);

// Case insensitive.
[[nodiscard]] bool contains(const std::string_view s,
                            const std::string_view pattern);

[[nodiscard]] std::string join(const auto &values, const char *separator) {
  if (values.empty()) return "";

  using T = std::decay_t<decltype(values)>;
  using value_type = T::value_type;

  std::ostringstream oss;
  std::copy(values.cbegin(), values.cend() - 1,
            std::ostream_iterator<value_type>(oss, separator));
  oss << *values.crbegin();
  return oss.str();
}

[[nodiscard]] std::string formatBytes(std::size_t bytes);
