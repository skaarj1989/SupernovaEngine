#include "MaterialEditor/LuaMaterialEditor.hpp"
#include "MaterialEditor/FunctionAvailability.hpp"
#include "sol/state.hpp"
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

  lua["countChannels"] = countChannels;
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

  struct GenTypeBundle {
    DataType base{DataType::Undefined};
    using Group = std::array<DataType, 4>;
    Group group;
  };
  // clang-format off
  lua.new_usertype<GenTypeBundle>("GenTypeBundle",
    "base", &GenTypeBundle::base,
    "group", &GenTypeBundle::group
  );
  // clang-format on

  lua["genBTypeBundle"] = GenTypeBundle{DataType::Bool, genBType};
  lua["genUTypeBundle"] = GenTypeBundle{DataType::UInt32, genUType};
  lua["genITypeBundle"] = GenTypeBundle{DataType::Int32, genIType};
  lua["genTypeBundle"] = GenTypeBundle{DataType::Float, genType};
  lua["genDTypeBundle"] = GenTypeBundle{DataType::Double, genDType};
}

void registerShaderType(sol::state &lua) {
  using rhi::ShaderType;
  // clang-format off
  lua.new_enum<ShaderType>("ShaderType", {
    MAKE_PAIR(ShaderType, Vertex),
    MAKE_PAIR(ShaderType, Fragment),
  });
  // clang-format on
}
void registerMaterialDomain(sol::state &lua) {
  using gfx::MaterialDomain;
  // clang-format off
  lua.new_enum<MaterialDomain>("MaterialDomain", {
    MAKE_PAIR(MaterialDomain, Surface),
    MAKE_PAIR(MaterialDomain, PostProcess),
  });
  // clang-format on
}

void registerAttribute(sol::state &lua) {
  // clang-format off
  lua.new_enum<Attribute>("Attribute", {
    MAKE_PAIR(Attribute, Position),
    MAKE_PAIR(Attribute, TexCoord0),
    MAKE_PAIR(Attribute, TexCoord1),
    MAKE_PAIR(Attribute, Normal),
    MAKE_PAIR(Attribute, Color),
  });
  // clang-format on
}
void registerBuiltInConstant(sol::state &lua) {
  // clang-format off
  lua.new_enum<BuiltInConstant>("BuiltInConstant", {
    MAKE_PAIR(BuiltInConstant, CameraPosition),
    MAKE_PAIR(BuiltInConstant, ScreenTexelSize),
    MAKE_PAIR(BuiltInConstant, AspectRatio),

    MAKE_PAIR(BuiltInConstant, ModelMatrix),
    MAKE_PAIR(BuiltInConstant, FragPosWorldSpace),
    MAKE_PAIR(BuiltInConstant, FragPosViewSpace),

    MAKE_PAIR(BuiltInConstant, ViewDir),
  });
  // clang-format on
}
void registerBuiltInSampler(sol::state &lua) {
  // clang-format off
  lua.new_enum<BuiltInSampler>("BuiltInSampler", {
    MAKE_PAIR(BuiltInSampler, SceneDepth),
    MAKE_PAIR(BuiltInSampler, SceneColor),
  });
  // clang-format on
}

void registerSurface(sol::state &lua) {
  // clang-format off
  using Surface = gfx::Material::Surface;
  lua.new_usertype<Surface>("Surface",
    sol::no_constructor,

    "shadingModel", &Surface::shadingModel,
    "blendMode", &Surface::blendMode,
    "decalBlendMode", &Surface::decalBlendMode,
    "lightingMode", &Surface::lightingMode,
    "cullMode", &Surface::cullMode
  );
  // clang-format on
}

void registerFunctionAvailabilityHelpers(sol::state &lua) {
#define REGISTER_FUNCTION(f) lua[#f] = f

  REGISTER_FUNCTION(vertexShaderOnly);
  REGISTER_FUNCTION(fragmentShaderOnly);
  REGISTER_FUNCTION(surfaceOnly);
  REGISTER_FUNCTION(postProcessOnly);

#undef REGISTER_FUNCTION
}

template <typename T>
  requires std::is_scoped_enum_v<T>
bool isOutOfRange(const T v) {
  return std::to_underlying(v) >= std::to_underlying(T::COUNT);
}

using Parameter = ScriptedFunction::Data::Parameter;
using ParameterVariant = TransientVariant;

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
    if (auto e = s.get<sol::optional<BuiltInSampler>>(); e) {
      out =
        !isOutOfRange(*e) ? *e : Result{std::unexpected{"Enum out of range."}};
    } else if (auto tp = s.get<sol::optional<TextureParam>>(); tp) {
      out = *tp;
    } else {
      out = std::unexpected{
        "Invalid value, expected: BuiltInSampler/TextureParam."};
    }
  }

  if (!out) throw std::runtime_error{out.error()};

  return out.value();
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
  // clang-format on

  static constexpr auto makeGuid = [](const std::string &name) {
    return entt::hashed_string{name.c_str()}.value();
  };

  // clang-format off
  lua.new_usertype<ScriptedFunction>("FunctionInfo", 
    sol::call_constructor,
    sol::factories(
      [](const sol::table &t) {
        auto name = t["name"].get<std::string>();

        return ScriptedFunction{
          .id = t["guid"].get_or(makeGuid(name)),
          .data = {
            .category = t.get_or<std::string>("category", "(Undefined)"),
            .name = std::move(name),
            .description = t.get_or<std::string>("description", ""),
            .signature = extractSignatures(t),
            .args = t.get_or<ScriptedFunction::Data::Parameters>("args", {}),
            .isEnabledCb =
              [f = getFunction(t, "isEnabled")](
                const gfx::Material::Surface *surface,
                const rhi::ShaderType shaderType) {
                try {
                  if (f) {
                    auto result = std::invoke(*f, surface, shaderType);
                    return result.valid() ? result.get<bool>() : false;
                  }
                } catch (const std::exception &) {
                  // ...
                }
                return true;
              },
            .getReturnTypeCb =
              [f = getFunction(t, "getReturnType")](
                std::span<const DataType> args) {
                try {
                  if (f) {
                    auto result = std::invoke(*f, args);
                    return result.valid() ? result.get<DataType>()
                                          : DataType::Undefined;
                  }
                } catch (const std::exception &) {
                  // ...
                }
                return DataType::Undefined;
              },
          },
        };
    })
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
  registerSurface(lua);

  registerFunctionAvailabilityHelpers(lua);

  // clang-format off
  lua.new_usertype<TextureParam>("TextureParam",
    sol::call_constructor,
    sol::factories([] { return TextureParam{}; })
  );
  // clang-format on

  registerFunctionInfo(lua);
}

std::expected<ScriptedFunction, std::string>
loadFunction(const std::filesystem::path &p, sol::state &lua) {
  auto result = lua.safe_script_file(p.string(), &sol::script_pass_on_error);
  if (!result.valid()) {
    const sol::error err = result;
    return std::unexpected{err.what()};
  }

  if (auto data = result.get<std::optional<ScriptedFunction>>(); data) {
    return std::move(*data);
  } else {
    return std::unexpected{
      std::format("{}: Invalid data structure.", p.generic_string()),
    };
  }
}
