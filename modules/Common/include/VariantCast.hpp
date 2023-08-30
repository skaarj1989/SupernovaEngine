#pragma once

#include <variant>

// https://stackoverflow.com/a/47204507

template <class... Args> struct variant_cast_proxy {
  std::variant<Args...> v;

  template <class... ToArgs> operator std::variant<ToArgs...>() const {
    return std::visit([](auto &&arg) -> std::variant<ToArgs...> { return arg; },
                      v);
  }
};

template <class... Args>
auto variant_cast(const std::variant<Args...> &v)
  -> variant_cast_proxy<Args...> {
  return {v};
}
