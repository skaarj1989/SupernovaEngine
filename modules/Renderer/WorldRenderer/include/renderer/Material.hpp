#pragma once

#include "rhi/CullMode.hpp"
#include "MaterialProperty.hpp"
#include "TextureManager.hpp"
#include <map>

namespace gfx {

enum class MaterialDomain : uint8_t {
  Surface, // also a Decal.
  PostProcess,
};
enum class ShadingModel : uint8_t { Unlit, Lit };
enum class BlendMode : uint8_t {
  Opaque,
  Masked,
  Transparent,
  Add,
  Modulate,
};
enum class LightingMode : uint8_t { Default, Transmission };

enum class BlendOp : uint8_t { Keep, Replace, Blend };
struct DecalBlendMode {
  BlendOp normal{BlendOp::Keep};
  BlendOp emissive{BlendOp::Replace};
  BlendOp albedo{BlendOp::Replace};
  BlendOp metallicRoughnessAO{BlendOp::Replace};
};

enum class MaterialFlags {
  None = 0,

  // Surface flags:
  // see shaders/MaterialDefines.glsl

  CastShadow = 1 << 0,
  ReceiveShadow = 1 << 1,

  SurfaceDefault = CastShadow | ReceiveShadow,

  // Postprocess flags:

  Enabled = 1 << 2,
};

struct TextureInfo {
  rhi::TextureType type{rhi::TextureType::Undefined};
  std::shared_ptr<rhi::Texture> texture;

  [[nodiscard]] auto isValid() const {
    return type != rhi::TextureType::Undefined && texture && *texture;
  }

  friend bool operator==(const TextureInfo &a, const TextureInfo &b) {
    return a.texture == b.texture;
  }

  template <class Archive> void save(Archive &archive) const {
    archive(serialize(texture));
  }
  template <class Archive> void load(Archive &archive) {
    std::optional<std::string> path;
    archive(path);
    if (path) {
      texture = loadResource<TextureManager>(*path);
      if (texture) type = texture->getType();
    }
  }
};
using TextureResources = std::map<std::string, TextureInfo, std::less<>>;

[[nodiscard]] bool operator==(const TextureResources &,
                              const TextureResources &);

struct PropertyLayout {
  struct MemberInfo {
    uint8_t offset;
    uint8_t size;
  };
  std::vector<MemberInfo> members;
  std::size_t stride{0};
};

class Material {
public:
  struct Surface {
    ShadingModel shadingModel{ShadingModel::Lit};
    BlendMode blendMode{BlendMode::Opaque};
    DecalBlendMode decalBlendMode;
    LightingMode lightingMode{LightingMode::Default};
    rhi::CullMode cullMode{rhi::CullMode::Back};
  };
  struct Blueprint {
    std::optional<Surface> surface;
    std::vector<Property> properties;
    TextureResources defaultTextures;

    struct Code {
      using Define = std::pair<std::string, int32_t>;
      using Defines = std::vector<Define>;
      Defines defines;
      std::string includes;
      std::string source;
    };
    Code userVertCode;
    Code userFragCode;

    MaterialFlags flags{MaterialFlags::None};
  };

  Material(const Material &) = delete;
  Material(Material &&) noexcept = default;
  virtual ~Material() = default;

  Material &operator=(const Material &) = delete;
  Material &operator=(Material &&) noexcept = default;

  bool friend operator<(const Material &, const Material &);

  [[nodiscard]] std::string_view getName() const;
  [[nodiscard]] std::size_t getHash() const;
  [[nodiscard]] const Blueprint &getBlueprint() const;
  [[nodiscard]] const PropertyLayout &getPropertyLayout() const;

  class Builder {
  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) noexcept = delete;

    Builder &setName(const std::string_view);

    Builder &setBlueprint(Blueprint);

    Builder &setDomain(MaterialDomain);
    Builder &setSurface(Surface);
    Builder &setPostProcess();

    Builder &addProperty(const Property &);
    Builder &addSampler(rhi::TextureType, const std::string &alias,
                        std::shared_ptr<rhi::Texture>);

    Builder &setUserVertCode(Blueprint::Code);
    Builder &setUserFragCode(Blueprint::Code);

    Builder &setFlags(MaterialFlags);

    [[nodiscard]] Material build();

  private:
    std::string m_name;
    Blueprint m_blueprint;
  };

private:
  Material(std::string &&name, std::size_t hash, const Blueprint &);

private:
  std::string m_name;
  const std::size_t m_hash{0u};

  Blueprint m_blueprint;
  PropertyLayout m_propertyLayout;
};

[[nodiscard]] MaterialDomain getDomain(const Material &);
[[nodiscard]] MaterialDomain getDomain(const Material::Blueprint &);

[[nodiscard]] bool isSurface(const Material &);
[[nodiscard]] bool isSurface(const Material::Blueprint &);

[[nodiscard]] const Material::Surface &getSurface(const Material &);

[[nodiscard]] bool hasProperties(const Material &);

[[nodiscard]] const char *toString(MaterialDomain);
[[nodiscard]] const char *toString(ShadingModel);
[[nodiscard]] const char *toString(BlendMode);
[[nodiscard]] const char *toString(LightingMode);

[[nodiscard]] std::string toString(MaterialFlags);
[[nodiscard]] std::vector<const char *> getValues(MaterialFlags);

} // namespace gfx

template <> struct has_flags<gfx::MaterialFlags> : std::true_type {};
