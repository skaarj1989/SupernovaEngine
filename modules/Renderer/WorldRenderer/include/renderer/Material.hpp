#pragma once

#include "rhi/ShaderType.hpp"
#include "rhi/CullMode.hpp"
#include "MaterialProperty.hpp"
#include "TextureResources.hpp"

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

struct PropertyLayout {
  struct MemberInfo {
    uint8_t offset;
    uint8_t size;
  };
  std::vector<MemberInfo> members;
  uint32_t stride{0};
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
    std::map<rhi::ShaderType, Code> userCode;

    MaterialFlags flags{MaterialFlags::None};
  };

  Material(const Material &) = delete;
  Material(Material &&) noexcept = default;
  virtual ~Material() = default;

  Material &operator=(const Material &) = delete;
  Material &operator=(Material &&) noexcept = default;

  bool friend operator<(const Material &, const Material &);

  using Hash = std::size_t;

  [[nodiscard]] std::string_view getName() const;
  [[nodiscard]] Hash getHash() const;
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

    Builder &setDomain(const MaterialDomain);
    Builder &setSurface(Surface);
    Builder &setPostProcess();

    Builder &addProperty(const Property &);
    Builder &addSampler(const rhi::TextureType, const std::string &alias,
                        std::shared_ptr<rhi::Texture>);

    Builder &setUserCode(const rhi::ShaderType, Blueprint::Code);

    Builder &setFlags(const MaterialFlags);

    [[nodiscard]] Material build();

  private:
    std::string m_name;
    Blueprint m_blueprint;
  };

private:
  Material(std::string &&name, const Hash, const Blueprint &);

private:
  std::string m_name;
  const Hash m_hash{0u};

  Blueprint m_blueprint;
  PropertyLayout m_propertyLayout;
};

[[nodiscard]] MaterialDomain getDomain(const Material &);
[[nodiscard]] MaterialDomain getDomain(const Material::Blueprint &);

[[nodiscard]] bool isSurface(const Material &);
[[nodiscard]] bool isSurface(const Material::Blueprint &);

[[nodiscard]] const Material::Surface &getSurface(const Material &);

[[nodiscard]] bool hasProperties(const Material &);

[[nodiscard]] const char *toString(const MaterialDomain);
[[nodiscard]] const char *toString(const ShadingModel);
[[nodiscard]] const char *toString(const BlendMode);
[[nodiscard]] const char *toString(const LightingMode);

[[nodiscard]] std::string toString(const MaterialFlags);
[[nodiscard]] std::vector<const char *> getValues(const MaterialFlags);

} // namespace gfx

template <> struct has_flags<gfx::MaterialFlags> : std::true_type {};
