#include "LuaFileSystem.hpp"
#include "sol/state.hpp"
#include "os/FileSystem.hpp"

void registerFileSystem(sol::state &lua) {
  auto m = lua["FileSystem"].get_or_create<sol::table>();

  m["rootPath"] = [] { return os::FileSystem::getRoot().string(); };
  m["currentPath"] = [p = std::filesystem::current_path().string()] {
    return p;
  };
}
