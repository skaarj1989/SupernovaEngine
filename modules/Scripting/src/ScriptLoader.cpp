#include "ScriptLoader.hpp"
#include "os/FileSystem.hpp"
#include "spdlog/spdlog.h"

namespace {

[[nodiscard]] std::expected<std::string, std::string>
tryLoad(const std::filesystem::path &p) {
  const auto ext = os::FileSystem::getExtension(p);
  if (!ext) {
    return std::unexpected{"No extension."};
  } else if (ext != ".lua") {
    return std::unexpected{"Invalid extension."};
  }
  return os::FileSystem::readText(p);
}

} // namespace

ScriptLoader::result_type
ScriptLoader::operator()(const std::filesystem::path &p) const {
  if (auto script = tryLoad(p); script) {
    return std::make_shared<ScriptResource>(std::move(script.value()), p);
  } else {
    SPDLOG_ERROR("Script loading failed. {}", script.error());
    return {};
  }
}
