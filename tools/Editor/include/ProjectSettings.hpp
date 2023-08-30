#pragma once

#include <filesystem>
#include <expected>

struct ProjectSettings {
  std::string name;
  std::filesystem::path path;

  bool dirty{false};

  template <class Archive> void serialize(Archive &archive) { archive(name); }
};

[[nodiscard]] std::expected<ProjectSettings, std::string>
createProject(const std::filesystem::path &);
bool save(const ProjectSettings &);
[[nodiscard]] std::expected<ProjectSettings, std::string>
load(const std::filesystem::path &);
