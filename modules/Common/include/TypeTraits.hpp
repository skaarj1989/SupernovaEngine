#pragma once

#include <type_traits>

// https://stackoverflow.com/a/39550575

template <class T, class... Ts>
struct is_any : std::disjunction<std::is_same<T, Ts>...> {};

template <class T, class... Ts>
constexpr bool is_any_v = is_any<T, Ts...>::value;
