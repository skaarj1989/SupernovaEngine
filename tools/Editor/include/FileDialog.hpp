#pragma once

#include "os/FileSystem.hpp"
#include <functional>
#include <optional>

enum FileDialogFlags_ {
  FileDialogFlags_None = 0,
  FileDialogFlags_CreateDirectoryButton = 1 << 1,
  FileDialogFlags_DirectoryOnly = 1 << 2,
  FileDialogFlags_ResetOnClose = 1 << 3,
  FileDialogFlags_AskOverwrite = 1 << 4,
};
using FileDialogFlags = int32_t;

using EntryFilter =
  std::function<bool(const std::filesystem::directory_entry &)>;

constexpr auto makeExtensionFilter(const char *desiredExtension) {
  return [desiredExtension](const std::filesystem::directory_entry &entry) {
    const auto entryExtension = os::FileSystem::getExtension(entry);
    return entryExtension && *entryExtension == desiredExtension;
  };
};

struct FileDialogSettings {
  std::filesystem::path &dir;
  std::optional<std::filesystem::path> barrier{std::nullopt};
  EntryFilter entryFilter;
  std::optional<std::string_view> forceExtension;
  FileDialogFlags flags{FileDialogFlags_None};
};

[[nodiscard]] std::optional<std::filesystem::path>
showFileDialog(const char *name, const FileDialogSettings &);
