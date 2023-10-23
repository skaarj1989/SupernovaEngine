#include "MaterialEditor/LuaMaterialEditor.hpp"
#include "sol/state.hpp"

#include "MaterialEditor/ScriptedFunctionData.hpp"
#include "MaterialEditor/FunctionAvailability.hpp"

#include "LuaMath.hpp"

#include "entt/core/hashed_string.hpp"

#include <format>

#define MAKE_PAIR(EnumClass, Value)                                            \
  { #Value, EnumClass::Value }

namespace {

void registerDataType(sol::state &lua) {
  // clang-format off
  lua.new_enum<DataType>("DataType", {
                                       MAKE_PAIR(DataType, Undefined),

                                       MAKE_PAIR(DataType, Bool),
                                       MAKE_PAIR(DataType, BVec2),
                                       MAKE_PAIR(DataType, BVec3),
                                       MAKE_PAIR(DataType, BVec4),

                                       MAKE_PAIR(DataType, UInt32),
                                       MAKE_PAIR(DataType, UVec2),
                                       MAKE_PAIR(DataType, UVec3),
                                       MAKE_PAIR(DataType, UVec4),

                                       MAKE_PAIR(DataType, Int32),
                                       MAKE_PAIR(DataType, IVec2),
                                       MAKE_PAIR(DataType, IVec3),
                                       MAKE_PAIR(DataType, IVec4),

                                       MAKE_PAIR(DataType, Float),
                                       MAKE_PAIR(DataType, Vec2),
                                       MAKE_PAIR(DataType, Vec3),
                                       MAKE_PAIR(DataType, Vec4),

                                       MAKE_PAIR(DataType, Double),
                                       MAKE_PAIR(DataType, DVec2),
                                       MAKE_PAIR(DataType, DVec3),
                                       MAKE_PAIR(DataType, DVec4),

                                       MAKE_PAIR(DataType, Mat2),
                                       MAKE_PAIR(DataType, Mat3),
                                       MAKE_PAIR(DataType, Mat4),

                                       MAKE_PAIR(DataType, Sampler2D),
                                       MAKE_PAIR(DataType, SamplerCube),
                                     });
  // clang-format on

  lua["isScalar"] = isScalar;
  lua["isVector"] = isVector;
  lua["isMatrix"] = isMatrix;
  lua["isSampler"] = isSampler;

  lua["getBaseDataType"] = getBaseDataType;

  lua["countChannels"] = sol::resolve<int32_t(DataType)>(countChannels);
  lua["countColumns"] = countColumns;

  lua["constructVectorType"] = constructVectorType;

  using enum DataType;

  lua["bvec"] = std::array{BVec2, BVec3, BVec4};
  const auto genBType = std::array{Bool, BVec2, BVec3, BVec4};
  lua["genBType"] = genBType;

  lua["uvec"] = std::array{UVec2, UVec3, UVec4};
  const auto genUType = std::array{UInt32, UVec2, UVec3, UVec4};
  lua["genUType"] = genUType;

  lua["ivec"] = std::array{IVec2, IVec3, IVec4};
  const auto genIType = std::array{Int32, IVec2, IVec3, IVec4};
  lua["genIType"] = genIType;

  lua["vec"] = std::array{Vec2, Vec3, Vec4};
  const auto genType = std::array{Float, Vec2, Vec3, Vec4};
  lua["genType"] = genType;

  lua["dvec"] = std::array{DVec2, DVec3, DVec4};
  const auto genDType = std::array{Double, DVec2, DVec3, DVec4};
  lua["genDType"] = genDType;

  // std::pair<DataType, std::array<DataType>> does not work.
  // sol2 sees the mentioned pair as std::array (.second).
  struct CustomPair {
    DataType base{DataType::Undefined};
    using Group = std::array<DataType, 4>;
    Group group;
  };
  // clang-format off
  lua.new_usertype<CustomPair>("CustomPair",
    "base", &CustomPair::base,
    "group", &CustomPair::group
  );
  // clang-format on

  lua["genBTypeBundle"] = CustomPair{DataType::Bool, genBType};
  lua["genUTypeBundle"] = CustomPair{DataType::UInt32, genUType};
  lua["genITypeBundle"] = CustomPair{DataType::Int32, genIType};
  lua["genTypeBundle"] = CustomPair{DataType::Float, genType};
  lua["genDTypeBundle"] = CustomPair{DataType::Double, genDType};
}

void registerShaderType(sol::state &lua) {
  using rhi::ShaderType;
  lua.new_enum<ShaderType>("ShaderType", {
                                           MAKE_PAIR(ShaderType, Vertex),
                                           MAKE_PAIR(ShaderType, Fragment),
                                         });
}
void registerMaterialDomain(sol::state &lua) {
  using gfx::MaterialDomain;
  lua.new_enum<MaterialDomain>("MaterialDomain",
                               {
                                 MAKE_PAIR(MaterialDomain, Surface),
                                 MAKE_PAIR(MaterialDomain, PostProcess),
                               });
}

void registerBlueprint(sol::state &lua) {
  lua.new_usertype<gfx::Material::Blueprint>("Blueprint", sol::no_constructor);

  lua["getDomain"] =
    sol::resolve<gfx::MaterialDomain(const gfx::Material::Blueprint &)>(
      gfx::getDomain);
}

void registerAttribute(sol::state &lua) {
  lua.new_enum<Attribute>("Attribute", {
                                         MAKE_PAIR(Attribute, Position),
                                         MAKE_PAIR(Attribute, TexCoord0),
                                         MAKE_PAIR(Attribute, TexCoord1),
                                         MAKE_PAIR(Attribute, Normal),
                                         MAKE_PAIR(Attribute, Color),
                                       });
}
void registerBuiltInConstant(sol::state &lua) {
  lua.new_enum<BuiltInConstant>("BuiltInConstant",
                                {
                                  MAKE_PAIR(BuiltInConstant, CameraPosition),
                                  MAKE_PAIR(BuiltInConstant, ScreenTexelSize),
                                  MAKE_PAIR(BuiltInConstant, AspectRatio),

                                  MAKE_PAIR(BuiltInConstant, ModelMatrix),
                                  MAKE_PAIR(BuiltInConstant, FragPosWorldSpace),
                                  MAKE_PAIR(BuiltInConstant, FragPosViewSpace),

                                  MAKE_PAIR(BuiltInConstant, ViewDir),
                                });
}
void registerBuiltInSampler(sol::state &lua) {
  lua.new_enum<BuiltInSampler>("BuiltInSampler",
                               {
                                 MAKE_PAIR(BuiltInSampler, SceneDepth),
                                 MAKE_PAIR(BuiltInSampler, SceneColor),
                               });
}

void registerFunctionAvailabilityHelpers(sol::state &lua) {
#define REGISTER_FUNCTION(f) lua[#f] = f

  REGISTER_FUNCTION(vertexShaderOnly);
  REGISTER_FUNCTION(fragmentShaderOnly);
  REGISTER_FUNCTION(surfaceOnly);
  REGISTER_FUNCTION(postProcessOnly);

#undef REGISTER_FUNCTION
}

template <typename T> bool isOutOfRange(T v) {
  return std::to_underlying(v) >= std::to_underlying(T::COUNT);
}

using Parameter = ScriptedFunctionData::Parameter;
using ParameterVariant = Parameter::Variant;

template <typename T>
[[nodiscard]] std::expected<ParameterVariant, std::string> tryGet(auto &t) {
  if (auto temp = t.template get<sol::optional<T>>(); temp) {
    if constexpr (std::is_enum_v<T>) {
      if (isOutOfRange(*temp)) {
        return std::unexpected{"Enum value out of range."};
      }
    }
    return *temp;
  }
  return std::unexpected{
    std::format("Invalid type, expected: {}.", typeid(T).name()),
  };
}

[[nodiscard]] auto getFunction(const sol::table &t,
                               const std::string_view name) {
  auto f = t[name];
  return f.get_type() == sol::type::function
           ? std::optional{f.get<sol::function>()}
           : std::nullopt;
};

[[nodiscard]] auto extractSignatures(const sol::table &t) {
  auto signatures = t.get_or<std::vector<std::string>>("signatures", {});
  if (signatures.empty()) {
    if (const auto signature = t.get<std::optional<std::string>>("signature");
        signature) {
      signatures.push_back(*signature);
    }
  }

  std::ostringstream oss;
  for (const auto &str : signatures) {
    std::ostream_iterator<std::string>{oss, "\n"} = str;
  }
  return oss.str();
}

[[nodiscard]] auto extractDefaultValue(const sol::table &t) {
  // enum in sol2 is just a number, so the following code:
  // defaultValue = Attribute.TexCoord0
  // would become a ValueVariant of uint32_t

  using Result = std::expected<ParameterVariant, std::string>;
  Result out;

  if (auto v = t["value"]; v.valid()) {
    out = tryGet<ValueVariant>(v);
  } else if (auto a = t["attribute"]; a.valid()) {
    out = tryGet<Attribute>(a);
  } else if (auto c = t["constant"]; c.valid()) {
    out = tryGet<BuiltInConstant>(c);
  } else if (auto s = t["sampler"]; s.valid()) {
    if (auto temp = s.get<sol::optional<BuiltInSampler>>(); temp) {
      out = !isOutOfRange(*temp)
              ? *temp
              : Result{std::unexpected{"Enum out of range."}};
    } else if (auto temp = s.get<sol::optional<TextureParam>>(); temp) {
      out = *temp;
    } else {
      out = std::unexpected{
        "Invalid value, expected: BuiltInSampler/TextureParam."};
    }
  }

  if (!out) throw std::runtime_error{out.error()};

  return out.value();
};

struct ScriptedFunctionDataEx : ScriptedFunctionData {
  uint32_t guid;
};

void registerFunctionInfo(sol::state &lua) {
  // clang-format off
  lua.new_usertype<Parameter>("Parameter",
    sol::call_constructor,
    sol::factories(
      [] { return Parameter{}; },
      [](const sol::table &t) {
        return Parameter{
          .name = t.get<std::string>("name"),
          .description = t.get_or<std::string>("description", ""),
          .defaultValue = extractDefaultValue(t),
        };
      }
    ),

    sol::meta_function::to_string, []{ return "Parameter"; }
  );

  static constexpr auto makeGuid = [](const std::string &name) {
    return entt::hashed_string{name.c_str()}.value();
  };

  lua.new_usertype<ScriptedFunctionDataEx>("FunctionInfo", 
    sol::call_constructor,
    sol::factories(
      [](const sol::table &t) {
        auto name = t["name"].get<std::string>();

        ScriptedFunctionDataEx data{.guid = t["guid"].get_or(makeGuid(name))};
        data.category = t.get_or<std::string>("category", "(Undefined)");
        data.name = std::move(name);
        data.description = t.get_or<std::string>("description", "");
        data.signature = extractSignatures(t);

        data.args = t.get_or<ScriptedFunctionData::Parameters>("args", {});
        data.isEnabledCb = [f = getFunction(t, "isEnabled")](
                             const rhi::ShaderType shaderType,
                             const gfx::Material::Blueprint &blueprint) {
          try {
            if (f) {
              auto result = std::invoke(*f, shaderType, blueprint);
              return result.valid() ? result.get<bool>() : false;
            }
          } catch (const std::exception &e) {
            // ...
          }
          return true;
        };
        data.getReturnTypeCb = [f = getFunction(t, "getReturnType")]
                                 (std::span<const DataType> args) {
          try {
            if (f) {
              auto result = std::invoke(*f, args);
              return result.valid() ? result.get<DataType>()
                                    : DataType::Undefined;
            }
          }
          catch (const std::exception &e) {
            // ...
          }
          return DataType::Undefined;
        };
       
        return data;
      }
    )
  );
  // clang-format on
}

} // namespace

void registerMaterialNodes(sol::state &lua) {
  registerMath(lua);
  registerDataType(lua);

  registerAttribute(lua);
  registerBuiltInConstant(lua);
  registerBuiltInSampler(lua);

  registerShaderType(lua);
  registerMaterialDomain(lua);
  registerBlueprint(lua);

  registerFunctionAvailabilityHelpers(lua);

  // clang-format off
  lua.new_usertype<TextureParam>("TextureParam",
    sol::call_constructor,
    sol::factories([] { return TextureParam{}; })
  );
  // clang-format on

  registerFunctionInfo(lua);
}

std::expected<std::pair<uint32_t, ScriptedFunctionData>, std::string>
loadFunction(const std::filesystem::path &p, sol::state &lua) {
  auto result = lua.safe_script_file(p.string(), &sol::script_pass_on_error);
  if (!result.valid()) {
    const sol::error err = result;
    return std::unexpected{err.what()};
  }

  if (auto data = result.get<std::optional<ScriptedFunctionDataEx>>(); data) {
    return std::pair{data->guid, std::move(*data)};
  } else {
    return std::unexpected{
      std::format("{}: Invalid data structure.", p.generic_string()),
    };
  }
}
