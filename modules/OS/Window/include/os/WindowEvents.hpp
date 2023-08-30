#pragma once

#include "os/InputEvents.hpp"
#include "glm/ext/vector_uint2.hpp"
#include <set>
#include <filesystem>

namespace os {

struct MoveWindowEvent {
  glm::ivec2 position;
};
struct ResizeWindowEvent {
  glm::uvec2 size; // Client size.
};
struct CloseWindowEvent {};

struct DropFilesEvent {
  glm::ivec2 position;
  std::set<std::filesystem::path> filenames;
};

} // namespace os
