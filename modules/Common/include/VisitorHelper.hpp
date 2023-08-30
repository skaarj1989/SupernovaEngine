#pragma once

// Taken from: https://en.cppreference.com/w/cpp/utility/variant/visit#Example

template <class... Ts> struct Overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> Overload(Ts...) -> Overload<Ts...>;
