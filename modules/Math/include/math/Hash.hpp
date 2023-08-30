#pragma once

#include <functional> // hash

template <typename T, typename... Rest>
inline void hashCombine(std::size_t &seed, const T &v, Rest &&...rest) {
  // https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hashCombine(seed, rest), ...);
}
