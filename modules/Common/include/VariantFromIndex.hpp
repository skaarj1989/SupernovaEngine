#pragma once

#include <variant>
#include <utility> // in_place_index
#include <format>

// https://stackoverflow.com/a/60567091/4265215

template <class Variant, std::size_t I = 0>
Variant variant_from_index(std::size_t index) {
  if constexpr (I >= std::variant_size_v<Variant>) {
    throw std::runtime_error{
      std::format("Variant index {} out of bounds", I + index)};
  } else {
    return index == 0 ? Variant{std::in_place_index<I>}
                      : variant_from_index<Variant, I + 1>(index - 1);
  }
}
