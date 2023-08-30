#pragma once

#include "assimp/types.h"
#include <string>

struct aiStringHash {
  // Heterogeneous lookup is never used across the project.
  // using is_transparent = void;

  std::size_t operator()(const aiString &s) const noexcept {
    return std::hash<std::string_view>{}(std::string_view{s.C_Str(), s.length});
  }
};
