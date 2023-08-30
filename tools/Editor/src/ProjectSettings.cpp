#include "ProjectSettings.hpp"
#include "os/FileSystem.hpp"
#include "cereal/archives/json.hpp"

std::expected<ProjectSettings, std::string>
createProject(const std::filesystem::path &p) {
  ProjectSettings settings{
    .name = p.stem().string(),
    .path = p,
  };
  if (save(settings)) {
    return settings;
  } else {
    return std::unexpected{"Could not save project file."};
  }
}
bool save(const ProjectSettings &settings) {
  std::ostringstream oss;
  {
    cereal::JSONOutputArchive archive{oss};
    archive(settings);
  }
  if (os::FileSystem::saveText(settings.path, oss.str())) {
    os::FileSystem::setRoot(settings.path.parent_path());
    return true;
  }
  return false;
}
std::expected<ProjectSettings, std::string>
load(const std::filesystem::path &p) {
  if (auto text = os::FileSystem::readText(p); text) {
    ProjectSettings settings{.path = std::filesystem::absolute(p)};
    try {
      std::istringstream iss{*text};
      {
        cereal::JSONInputArchive archive{iss};
        archive(settings);
      }
      os::FileSystem::setRoot(settings.path.parent_path());
    } catch (const std::exception &e) {
      return std::unexpected{e.what()};
    }
    return settings;
  } else {
    return std::unexpected{text.error()};
  }
}
