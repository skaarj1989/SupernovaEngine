#pragma once

#include <filesystem>

namespace std::filesystem {

template <class Archive> string save_minimal(const Archive &, const path &p) {
  return p.string();
}
template <class Archive>
void load_minimal(const Archive &, path &p, const string &value) {
  p = value;
}

} // namespace std::filesystem
