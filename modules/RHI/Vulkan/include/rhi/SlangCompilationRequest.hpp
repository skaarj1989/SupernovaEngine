#pragma once

#include <string>
#include <unordered_map>

namespace rhi {

struct SlangCompilationRequest {
  std::string moduleName;

  using Defines = std::unordered_map<std::string, std::string>;
  Defines defines;

  template <typename T>
  inline SlangCompilationRequest &addDefine(const std::string &name, T &&v) {
    defines[name] = std::to_string(v);
    return *this;
  }
};

} // namespace rhi
