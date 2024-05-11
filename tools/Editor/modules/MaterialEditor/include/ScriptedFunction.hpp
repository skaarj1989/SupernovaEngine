#pragma once

#include "renderer/Material.hpp"
#include "TransientVariant.hpp"

struct ScriptedFunction {
  using ID = std::size_t;
  struct Data {
    std::string category;
    std::string name;
    std::string description;
    std::string signature;

    struct Parameter {
      std::string name;
      std::string description;
      TransientVariant defaultValue;
    };
    using Parameters = std::vector<Parameter>;
    Parameters args;

    using IsEnabled = std::function<bool(const gfx::Material::Surface *,
                                         const rhi::ShaderType)>;
    IsEnabled isEnabledCb;
    using GetReturnType = std::function<DataType(std::span<const DataType>)>;
    GetReturnType getReturnTypeCb;
  };

  struct Handle {
    ID id{0};
    const Data *data{nullptr};
  };

  ID id{0};
  Data data{};
};

// Key = GUID (Hashed name of ScriptedFunction::Data or overriden in lua).
using ScriptedFunctions =
  std::unordered_map<ScriptedFunction::ID,
                     std::unique_ptr<ScriptedFunction::Data>>;
