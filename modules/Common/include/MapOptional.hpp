#pragma once

#include <optional>

// https://stackoverflow.com/a/64500326

template <typename T, typename Fn>
[[nodiscard]] auto map(T &&o, Fn &&func)
  -> std::optional<decltype(func(*std::forward<T>(o)))> {
  if (o.has_value()) {
    return func(*std::forward<T>(o));
  }
  return std::nullopt;
}
