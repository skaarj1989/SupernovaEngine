#pragma once

#include "os/FileSystem.hpp"
#include "OzzFile.hpp"
#include "ozz/base/io/archive.h"

template <class T>
[[nodiscard]] std::expected<T, std::string>
tryLoad(const std::filesystem::path &p) {
  OzzFileWrapper file{os::FileSystem::mapFile(p)};
  if (!file.opened()) {
    return std::unexpected{"Could not open file"};
  }
  ozz::io::IArchive archive{&file};
  if (!archive.TestTag<T>()) {
    return std::unexpected{"Invalid tag"};
  }
  T resource{};
  archive >> resource;
  return resource;
}
