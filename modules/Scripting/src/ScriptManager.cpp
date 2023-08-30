#include "ScriptManager.hpp"
#include "LoaderHelper.hpp"

ScriptResourceHandle ScriptManager::load(const std::filesystem::path &p) {
  return ::load(*this, p, LoadMode::External);
}
