#pragma once

#include "DataType.hpp"
#include "rhi/ShaderType.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

struct UserFunctionData {
  std::string name;

  struct Parameter {
    DataType dataType{DataType::Undefined};
    std::string name;

    template <class Archive> void serialize(Archive &archive) {
      archive(dataType, name);
    }
  };
  std::vector<Parameter> inputs;
  DataType output{DataType::Undefined};

  rhi::ShaderStages shaderStages{rhi::ShaderStages::Vertex |
                                 rhi::ShaderStages::Fragment};

  std::vector<std::size_t> dependencies;
  std::string code;

  template <class Archive> void serialize(Archive &archive) {
    archive(name, inputs, output, shaderStages, dependencies, code);
  }
};

// Key = Hashed UserFunctionData.
using UserFunctions =
  std::unordered_map<std::size_t, std::unique_ptr<UserFunctionData>>;

[[nodiscard]] std::string buildDeclaration(const UserFunctionData &);
[[nodiscard]] std::string buildDefinition(const UserFunctionData &);

namespace std {

template <> struct hash<UserFunctionData> {
  std::size_t operator()(const UserFunctionData &) const noexcept;
};
template <> struct hash<UserFunctionData::Parameter> {
  std::size_t operator()(const UserFunctionData::Parameter &) const noexcept;
};

} // namespace std
