#pragma once

#include <vector>

template <typename T, typename... Args>
void insert(std::vector<T> &v, const Args &...args) {
  (v.insert(v.cend(), std::begin(args), std::end(args)), ...);
}
template <typename T, typename... Args>
[[nodiscard]] auto merge(const std::vector<T> &v, const Args &...args) {
  std::vector<T> out;
  out.reserve((v.size() + ... + args.size()));
  insert(out, v, args...);
  return out;
}
