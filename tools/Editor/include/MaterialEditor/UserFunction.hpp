#pragma once

#include "DataType.hpp"
#include "rhi/ShaderType.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

struct UserFunction {
  using ID = std::size_t;
  struct Data {
    std::string name;

    struct Parameter {
      std::string name;
      DataType dataType{DataType::Undefined};

      [[nodiscard]] bool isValid() const;

      template <class Archive> void serialize(Archive &archive) {
        archive(name, dataType);
      }
    };
    std::vector<Parameter> inputs;
    DataType output{DataType::Undefined};

    rhi::ShaderStages shaderStages{rhi::ShaderStages::Vertex |
                                   rhi::ShaderStages::Fragment};

    std::vector<ID> dependencies;
    std::string code;

    [[nodiscard]] bool isValid() const;

    template <class Archive> void serialize(Archive &archive) {
      archive(name, inputs, output, shaderStages, dependencies, code);
    }
  };

  struct Handle {
    ID id{0};
    const Data *data{nullptr};
  };
};

bool operator==(const UserFunction::Data &, const UserFunction::Data &);
bool operator==(const UserFunction::Data::Parameter &,
                const UserFunction::Data::Parameter &);

// Key = Hashed UserFunction::Data
using UserFunctions =
  std::unordered_map<UserFunction::ID, std::unique_ptr<UserFunction::Data>>;

[[nodiscard]] std::string buildDeclaration(const UserFunction::Data &);
[[nodiscard]] std::string buildDefinition(const UserFunction::Data &);

namespace std {

template <> struct hash<UserFunction::Data> {
  std::size_t operator()(const UserFunction::Data &) const noexcept;
};
template <> struct hash<UserFunction::Data::Parameter> {
  std::size_t operator()(const UserFunction::Data::Parameter &) const noexcept;
};

} // namespace std
