#include "renderer/MaterialLoader.hpp"
#include "os/FileSystem.hpp"
#include "rhi/json.hpp"
#include "renderer/jsonMaterial.hpp"
#include "ShaderCodeBuilder.hpp"
#include "spdlog/spdlog.h"

using namespace nlohmann;

namespace gfx {

namespace {

[[nodiscard]] std::string_view getDefaultFragCode() {
  static const auto kDefaultMaterialFragCode =
    ShaderCodeBuilder{}.buildFromFile("DefaultMaterial.frag.glsl");
  return kDefaultMaterialFragCode;
}

} // namespace

namespace offline {

namespace {

[[nodiscard]] auto parse(std::span<const std::string> values) {
#define CASE(Value)                                                            \
  if (s == #Value) {                                                           \
    out |= MaterialFlags::Value;                                               \
  }

  MaterialFlags out{MaterialFlags::None};
  for (const auto &s : values) {
    CASE(CastShadow)
    CASE(ReceiveShadow)
    CASE(Enabled)
  }

#undef CASE

  return out;
}

} // namespace

struct Material {
  std::string name;

  MaterialDomain domain{MaterialDomain::Surface};
  std::optional<gfx::Material::Surface> surface;

  using PropertyMap = std::map<std::string, Property::Value, std::less<>>;
  PropertyMap properties;

  struct Texture {
    rhi::TextureType type{rhi::TextureType::Undefined};
    std::filesystem::path path; // Relative to material root file.
  };
  using TextureMap = std::map<std::string, Texture, std::less<>>;
  TextureMap textures;

  struct Code {
    using Defines = std::vector<std::string>;
    Defines defines;
    using Includes = std::vector<std::filesystem::path>;
    Includes includes;
    // Relative to material root file.
    std::optional<std::filesystem::path> path;
  };
  Code vertexShader;
  Code fragmentShader;

  std::optional<MaterialFlags> flags;
};
void from_json(const json &j, Material::Texture &out) {
  j.at("type").get_to(out.type);
  j.at("path").get_to(out.path);
}
void from_json(const json &j, Material::Code &out) {
  out.defines = j.value("defines", Material::Code::Defines{});
  out.includes = j.value("includes", Material::Code::Includes{});
  if (auto p = j.value("path", std::filesystem::path{}); !p.empty()) {
    out.path = std::move(p);
  }
}
void from_json(const json &j, Material &out) {
  out.name = j.value("name", "");
  j.at("domain").get_to(out.domain);
  if (out.domain == MaterialDomain::Surface) {
    out.surface = {
      .shadingModel = j.value("shadingModel", ShadingModel::Lit),
      .blendMode = j.value("blendMode", BlendMode::Opaque),
      .decalBlendMode = j.value("decalBlendMode", DecalBlendMode{}),
      .lightingMode = j.value("lightingMode", LightingMode::Default),
      .cullMode = j.value("cullMode", rhi::CullMode::Back),
    };
  }

  /*
  "key": {
    "type": string { uint, int, float, vec2, vec4 }
    "value": variant (of above type)
  }
  */
  out.properties = j.value("properties", Material::PropertyMap{});

  /*
  "key": {
    "type": string { sampler2D, samplerCube }
    "path": string (relative to p, metadata file)
  }
  */
  out.textures = j.value("samplers", Material::TextureMap{});

  out.vertexShader = j.value("vertexShader", Material::Code{});
  out.fragmentShader = j.value("fragmentShader", Material::Code{});

  if (j.contains("flags")) {
    const auto values = j["flags"].get<std::vector<std::string>>();
    out.flags = parse(values);
  }
}

} // namespace offline

MaterialLoader::result_type
MaterialLoader::operator()(const std::filesystem::path &p,
                           TextureManager &textureManager) const {
  try {
    auto text = os::FileSystem::readText(p);
    if (!text) throw std::runtime_error{text.error()};
    const offline::Material materialMeta = json::parse(*text);

    auto builder = Material::Builder{};
    builder.setName(materialMeta.name);
    if (materialMeta.domain == MaterialDomain::Surface) {
      builder.setSurface(*materialMeta.surface);
    }
    for (const auto &[name, value] : materialMeta.properties) {
      builder.addProperty({.name = name, .value = value});
    }

    const auto dir = p.parent_path();

    for (const auto &[name, info] : materialMeta.textures) {
      TextureResourceHandle textureResource;
      if (!info.path.empty()) {
        textureResource = textureManager.load(dir / info.path);
      }
      builder.addSampler(info.type, name, textureResource.handle());
    }

    const auto assembleCustomShader =
      [&dir](const offline::Material::Code &info,
             std::string_view defaultCode = "") {
        Material::Blueprint::Code code{.defines = info.defines};
        if (info.path) {
          if (!info.includes.empty()) {
            ShaderCodeBuilder shaderCodeBuilder{dir};
            for (const auto &entry : info.includes) {
              shaderCodeBuilder.include(entry);
            }
            code.includes = shaderCodeBuilder.buildFromString("");
          }
          if (auto glsl = os::FileSystem::readText(dir / *info.path); glsl) {
            code.source = std::move(*glsl);
          }
        } else {
          code.source = defaultCode;
        }
        return code;
      };

    builder.setUserVertCode(assembleCustomShader(materialMeta.vertexShader))
      .setUserFragCode(assembleCustomShader(materialMeta.fragmentShader,
                                            getDefaultFragCode()));
    if (materialMeta.flags) {
      builder.setFlags(*materialMeta.flags);
    }
    return std::make_shared<MaterialResource>(builder.build(), p);
  } catch (const std::exception &e) {
    SPDLOG_WARN("{}: {}", os::FileSystem::relativeToRoot(p)->generic_string(),
                e.what());
    return nullptr;
  }
}

MaterialLoader::result_type
MaterialLoader::operator()(const std::string_view name,
                           Material &&material) const {
  return std::make_shared<MaterialResource>(std::move(material), name);
}

} // namespace gfx
