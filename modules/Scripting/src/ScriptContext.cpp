#include "ScriptContext.hpp"
#include "os/FileSystem.hpp"

ScriptContext::ScriptContext(sol::state &s)
    : lua{std::addressof(s)}, defaultEnv{*lua, sol::create, lua->globals()} {
  const auto projectLocalPath = os::FileSystem::getRoot() / "";
  auto packagePath = defaultEnv["package"]["path"];
  packagePath =
    packagePath.set(std::format("{}{}?.lua;", packagePath.get<std::string>(),
                                projectLocalPath.lexically_normal().string()));
}
