#pragma once

#include <filesystem>

namespace std::filesystem {

template <class Archive> string save_minimal(const Archive &, const path &in) {
  return in.generic_string();
}
template <class Archive>
void load_minimal(const Archive &, path &out, const string &value) {
  out = value;
}

} // namespace std::filesystem
