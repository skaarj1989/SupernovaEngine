#include "MaterialConverter.hpp"
#include "ScopedEnumFlags.hpp"
#include "os/FileSystem.hpp"

#include "ai2glm.hpp" // to_vec4
#include "assimp/GltfMaterial.h"
#ifdef _DEBUG
#  include "MaterialDebugInfo.hpp"
#endif
#include "aiStringHash.hpp"

#include "rhi/json.hpp"
#include "renderer/jsonMaterial.hpp"

#include <bit>     // popcount
#include <fstream> // ofstream
#include <format>

// https://assimp.sourceforge.net/lib_html/materials.html

namespace offline {

void to_json(nlohmann::ordered_json &j, const Material::Texture &in) {
  j = nlohmann::ordered_json{
    {"type", "sampler2D"},
    {"path", in.path},
  };
}
void to_json(nlohmann::ordered_json &j, const Material &in) {
  j = nlohmann::ordered_json{
    {"name", in.name},
    {"domain", gfx::MaterialDomain::Surface},
    {"shadingModel", in.shadingModel},
    {"blendMode", in.blendMode},
    {"lightingMode", in.lightingMode},
    {
      "cullMode",
      in.twoSided ? "None" : "Back",
    },
    {"properties", in.properties},
    {"samplers", in.textures},
    {
      "fragmentShader",
      {{"defines", in.userFragCodeDefines}},
    },
  };
}

} // namespace offline

enum class TextureFlags {
  None = 0,
  Metallic = 1 << aiTextureType_METALNESS,          // .b
  Roughness = 1 << aiTextureType_DIFFUSE_ROUGHNESS, // .g
  Occlusion = 1 << aiTextureType_LIGHTMAP,          // .r
};
template <> struct has_flags<TextureFlags> : std::true_type {};

namespace {

[[nodiscard]] std::optional<aiString> getName(const aiMaterial &material) {
  if (aiString name; material.Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
    return name;
  }
  return std::nullopt;
}
[[nodiscard]] std::optional<aiString> getAlphaMode(const aiMaterial &material) {
  if (aiString alphaMode;
      material.Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
    return alphaMode;
  }
  return std::nullopt;
}
[[nodiscard]] auto getShadingMode(const aiMaterial &material) {
  aiShadingMode sm{aiShadingMode_NoShading};
  material.Get(AI_MATKEY_SHADING_MODEL, sm);
  return sm;
}

[[nodiscard]] auto getBlendMode(const std::string_view alphaMode) {
  using enum gfx::BlendMode;
  if (alphaMode == "OPAQUE")
    return Opaque;
  else if (alphaMode == "MASK")
    return Masked;
  else if (alphaMode == "BLEND")
    return Transparent;

  assert(false);
  return Opaque;
}

void eraseForbiddenCharacters(std::string &s) {
  constexpr auto isForbidden = [](const char c) {
    return os::FileSystem::getForbiddenCharacters().contains(c);
  };
  std::erase_if(s, isForbidden);
}
[[nodiscard]] auto camelToSnake(std::string_view in) {
  std::string result;
  constexpr auto kNumUnderscores = 3;
  result.reserve(in.length() + kNumUnderscores);
  result.push_back(char(std::tolower(in.front())));

  for (const auto c : in.substr(1)) {
    if (std::isupper(c)) {
      result += std::format("_{}", char(std::tolower(c)));
    } else {
      result += c;
    }
  }
  return result;
}
[[nodiscard]] auto camelToScreamingSnake(std::string_view in) {
  auto out = camelToSnake(in);
  std::ranges::transform(out, out.begin(), ::toupper);
  return out;
}

[[nodiscard]] auto makeDefine(const char *key, const char *suffix = nullptr) {
  const auto n = camelToScreamingSnake(key);
  return !suffix ? std::format("HAS_{}", n)
                 : std::format("HAS_{}_{}", n, suffix);
}

class Decorator {
public:
  Decorator(const aiMaterial &src, offline::Material &target)
      : m_source{src}, m_target{target} {
    if (auto name = getName(m_source); name) {
      m_target.name = name->C_Str();
    }
    eraseForbiddenCharacters(m_target.name);

    if (auto alphaMode = getAlphaMode(m_source); alphaMode) {
      m_target.blendMode = getBlendMode(alphaMode->C_Str());
    }
    m_source.Get(AI_MATKEY_TWOSIDED, m_target.twoSided);

    _scanPackedTexture();
  }

  template <typename T>
  bool addProperty(const char *name, const char *key, uint32_t type,
                   uint32_t idx) {
    if (T v; m_source.Get(key, type, idx, v) == AI_SUCCESS) {
      const auto toPropertyValue = [&v] {
        if constexpr (std::is_same_v<T, aiColor4D>) {
          return to_vec4(v);
        } else {
          // nlohmann::json writes infinity as "null" which triggers an
          // exception in json::parse (MaterialLoader).
          return !glm::isinf(v) ? v : 0.0f;
        }
      };
      _add({
        .name = name,
        .variant = toPropertyValue(),
        .define = std::pair{makeDefine(name), 1},
      });
      return true;
    }
    return false;
  }
  bool addTexture(const char *name, aiTextureType type, uint32_t index) {
    aiString path;
    uint32_t uvIndex{0};
    if (m_source.GetTexture(type, index, &path, nullptr, &uvIndex) ==
        AI_SUCCESS) {
      assert(uvIndex == 0);
      if (m_packedTexture && m_packedTexture->first == path) {
        return false;
      }
      _add({
        .name = std::format("t_{}", name),
        .variant = offline::Material::Texture{.path = _makePath(path)},
        .define = std::pair{makeDefine(name, "TEXTURE"), 1},
      });
      return true;
    }
    return false;
  }

  void addPackedTexture() {
    if (!m_packedTexture) return;

    _add({
      .name = "t_Packed",
      .variant =
        offline::Material::Texture{.path = _makePath(m_packedTexture->first)},
      .define = std::pair{"PACKED_TEXTURE",
                          std::to_underlying(m_packedTexture->second)},
    });
  }

private:
  void _scanPackedTexture() {
    std::unordered_map<aiString, TextureFlags, aiStringHash> map;
    for (const auto type : {
           aiTextureType_METALNESS,
           aiTextureType_DIFFUSE_ROUGHNESS,
           aiTextureType_LIGHTMAP,
         }) {
      if (aiString p; m_source.GetTexture(type, 0, &p) != AI_FAILURE) {
        map[p] |= TextureFlags(1 << type);
      }
    }
    std::erase_if(map, [](const auto &p) {
      return std::popcount(static_cast<uint32_t>(p.second)) == 1;
    });
    assert(map.size() <= 1);

    if (map.size() == 1) {
      m_packedTexture = *map.begin();
    }
  }

  static std::string _makePath(const aiString &path) {
    std::filesystem::path p{path.C_Str()};
    p = "../textures" / p.filename();
    return p.generic_string();
  }

  struct Record {
    std::string name;
    using Variant =
      std::variant<gfx::Property::Value, offline::Material::Texture>;
    Variant variant;
    std::optional<gfx::Material::Blueprint::Code::Define> define;
  };

  void _add(const Record &record) {
    std::visit(
      Overload{
        [this, key = record.name](const gfx::Property::Value &v) {
          m_target.properties[key] = v;
        },
        [this, key = record.name](const offline::Material::Texture &v) {
          m_target.textures[key] = v;
        },
      },
      record.variant);
    if (record.define) {
      m_target.userFragCodeDefines.emplace(*record.define);
    }
  }

private:
  const aiMaterial &m_source;
  std::optional<std::pair<aiString, TextureFlags>> m_packedTexture;
  offline::Material &m_target;
};

void getBaseInfo(Decorator &decorator) {
  decorator.addProperty<aiColor4D>("baseColorFactor", AI_MATKEY_COLOR_DIFFUSE);

  decorator.addProperty<aiColor4D>("baseColorFactor", AI_MATKEY_BASE_COLOR);
  decorator.addTexture("BaseColor", aiTextureType_DIFFUSE, 0);
  decorator.addProperty<float>("alphaCutOff", AI_MATKEY_GLTF_ALPHACUTOFF);
  decorator.addTexture("Normal", aiTextureType_NORMALS, 0);
}
void getMetallicRoughnessWorkflowInfo(Decorator &decorator) {
  decorator.addTexture("BaseColor", AI_MATKEY_BASE_COLOR_TEXTURE);
  decorator.addTexture("Metallic", AI_MATKEY_METALLIC_TEXTURE);
  decorator.addProperty<float>("metallicFactor", AI_MATKEY_METALLIC_FACTOR);
  decorator.addTexture("Roughness", AI_MATKEY_ROUGHNESS_TEXTURE);
  decorator.addProperty<float>("roughnessFactor", AI_MATKEY_ROUGHNESS_FACTOR);
  decorator.addTexture("AmbientOcclusion", aiTextureType_LIGHTMAP, 0);

  decorator.addPackedTexture();
}
bool getTransmissionInfo(Decorator &decorator) {
  auto b = decorator.addProperty<float>("transmissionFactor",
                                        AI_MATKEY_TRANSMISSION_FACTOR);
  b |= decorator.addTexture("Transmission", AI_MATKEY_TRANSMISSION_TEXTURE);
  return b;
}
bool getVolumeInfo(Decorator &decorator) {
  auto b = decorator.addProperty<float>("thicknessFactor",
                                        AI_MATKEY_VOLUME_THICKNESS_FACTOR);
  b |= decorator.addTexture("Thickness", AI_MATKEY_VOLUME_THICKNESS_TEXTURE);
  b |= decorator.addProperty<float>("attenuationDistance",
                                    AI_MATKEY_VOLUME_ATTENUATION_DISTANCE);
  b |= decorator.addProperty<aiColor4D>("attenuationColor",
                                        AI_MATKEY_VOLUME_ATTENUATION_COLOR);
  return b;
}
void getEmissionInfo(Decorator &decorator) {
  decorator.addProperty<aiColor4D>("emissiveColorFactor",
                                   AI_MATKEY_COLOR_EMISSIVE);
  decorator.addTexture("EmissiveColor", aiTextureType_EMISSIVE, 0);
}

} // namespace

bool exportMaterial(const offline::Material &material,
                    const std::filesystem::path &dir) {
  std::filesystem::path p{dir / material.name};
  std::filesystem::create_directories(p);

  const auto filename = p / std::format("{}.material", material.name);
  if (std::ofstream f{filename}; f.is_open()) {
    nlohmann::ordered_json j{material};
    f << std::setw(2) << j.front() << std::endl;
    return true;
  }
  return false;
}

[[nodiscard]] offline::Material convert(const aiMaterial &src, uint32_t index) {
#ifdef _DEBUG
  const auto debugInfo = toString(src);
#endif
  offline::Material out{.name = std::format("Material_{}", index)};
  Decorator decorator{src, out};

  getBaseInfo(decorator);
  if (getShadingMode(src) == aiShadingMode_PBR_BRDF) {
    out.userFragCodeDefines.emplace("IS_PBR", 1);
    getMetallicRoughnessWorkflowInfo(decorator);
    auto useTransmission = getTransmissionInfo(decorator);
    useTransmission |= getVolumeInfo(decorator);
    if (useTransmission) {
      out.lightingMode = gfx::LightingMode::Transmission;
    }
  }
  getEmissionInfo(decorator);
  return out;
}

std::size_t exportMaterials(std::span<const offline::SubMesh> subMeshes,
                            const std::filesystem::path &dir) {
  std::filesystem::create_directories(dir);
  std::filesystem::create_directories(dir / "textures");

  const auto withMaterial = std::views::filter(
    [](const auto &subMesh) { return subMesh.material.has_value(); });
  const auto saveMaterial = [&dir](const offline::SubMesh &subMesh) {
    return exportMaterial(*subMesh.material, dir);
  };
  return std::ranges::count_if(subMeshes | withMaterial, saveMaterial);
}
