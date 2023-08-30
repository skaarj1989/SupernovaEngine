#pragma once

#include <vector>

template <typename T, typename... Args>
void insert(std::vector<T> &v, const Args &...args) {
  (v.insert(v.cend(), args.cbegin(), args.cend()), ...);
}
template <typename T, typename... Args>
[[nodiscard]] auto merge(const std::vector<T> &v, const Args &...args) {
  std::vector<T> out;
  out.reserve((v.size() + ... + args.size()));
  insert(out, v, args...);
  return out;
}
