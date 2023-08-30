#pragma once

#include "ValueVariant.hpp"
#include "Attribute.hpp"
#include "FrameBlockMember.hpp"
#include "CameraBlockMember.hpp"
#include "BuiltInConstants.hpp"
#include "TextureParam.hpp"

#include "rhi/ShaderType.hpp"
#include "renderer/Material.hpp"

struct ScriptedFunctionData {
  std::string category;
  std::string name;
  std::string description;
  std::string signature;

  struct Parameter {
    std::string name;
    std::string description;

    using Variant = std::variant<std::monostate, ValueVariant, Attribute,
                                 BuiltInConstant, BuiltInSampler, TextureParam>;
    Variant defaultValue;
  };
  using Parameters = std::vector<Parameter>;
  Parameters args;

  using IsEnabled = std::function<bool(const rhi::ShaderType,
                                       const gfx::Material::Blueprint &)>;
  IsEnabled isEnabledCb;
  using GetReturnType = std::function<DataType(std::span<const DataType>)>;
  GetReturnType getReturnTypeCb;
};

// Key = GUID (Hashed name of ScriptedFunctionData or overriden in lua).
using ScriptedFunctions =
  std::unordered_map<uint32_t, std::unique_ptr<ScriptedFunctionData>>;
