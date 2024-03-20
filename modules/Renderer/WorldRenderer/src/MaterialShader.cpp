#include "MaterialShader.hpp"
#include <format>

namespace gfx {

uint32_t adjustStride(uint32_t stride, uint32_t minOffsetAlignment);

namespace {

#define DECLARE_REGION(Name) "#region " #Name

constexpr auto kUserSamplersRegion = DECLARE_REGION(USER_SAMPLERS);
constexpr auto kUserPropertiesRegion = DECLARE_REGION(USER_PROPERTIES);
constexpr auto kUserModulesRegion = DECLARE_REGION(USER_MODULES);
constexpr auto kUserCodeRegion = DECLARE_REGION(USER_CODE);

#undef DECLARE_REGION

[[nodiscard]] auto buildPropertiesChunk(const PropertyLayout &layout,
                                        const std::vector<Property> &properties,
                                        std::size_t minAlignment) {
  std::ostringstream oss;
  for (const auto &p : properties) {
    std::ostream_iterator<std::string>{oss, "\n"} =
      std::format(R"(  {} {};)", toString(p.value), p.name);
  }
  const auto padding =
    adjustStride(layout.stride, minAlignment) - layout.stride;
  if (padding > 0) {
    oss << std::format("  float _pad[{}];", padding / sizeof(float));
  }
  return oss.str();
}

[[nodiscard]] auto getSamplerType(rhi::TextureType textureType) {
  switch (textureType) {
    using enum rhi::TextureType;

  case Texture2D:
    return "sampler2D";
  case TextureCube:
    return "samplerCube";
  }
  assert(false);
  return "";
}
[[nodiscard]] std::string buildSamplersChunk(const TextureResources &textures,
                                             uint32_t set,
                                             uint32_t firstBinding = 0) {
  assert(!textures.empty());

  std::ostringstream oss;
  for (auto lastBinding = firstBinding;
       const auto &[alias, textureInfo] : textures) {
    std::ostream_iterator<std::string>{oss, "\n"} =
      std::format(R"(layout(set = {}, binding = {}) uniform {} {};)", set,
                  lastBinding, getSamplerType(textureInfo.type), alias);
    ++lastBinding;
  }
  return oss.str();
}

void addSurface(ShaderCodeBuilder &builder, const Material::Surface &surface) {
  builder.addDefine("SHADING_MODEL", uint32_t(surface.shadingModel))
    .addDefine("BLEND_MODE", int32_t(surface.blendMode));
  if (surface.lightingMode == LightingMode::Transmission)
    builder.addDefine("IS_TRANSMISSIVE", 1);
}
void addSamplers(ShaderCodeBuilder &builder, const TextureResources &textures) {
  constexpr auto kUserTexturesSet = 3;
  builder.replace(kUserSamplersRegion,
                  !textures.empty()
                    ? buildSamplersChunk(textures, kUserTexturesSet)
                    : "/* no samplers */");
}
void addProperties(ShaderCodeBuilder &builder, const Material &material,
                   VkDeviceSize minOffsetAlignment) {
  if (const auto &properties = material.getBlueprint().properties;
      !properties.empty()) {
    builder.addDefine("HAS_PROPERTIES", 1)
      .replace(kUserPropertiesRegion,
               buildPropertiesChunk(material.getPropertyLayout(), properties,
                                    minOffsetAlignment));
  } else {
    builder.replace(kUserPropertiesRegion, "/* no properties */");
  }
}

enum class FileIndex {
  Main,
  UserModule = 1,
};
[[nodiscard]] auto makeCodePatch(const FileIndex fileIndex,
                                 const std::string_view code) {
  return !code.empty()
           ? std::format("#line 1 {}\n{}", std::to_underlying(fileIndex), code)
           : "";
}

void addCode(ShaderCodeBuilder &builder,
             const Material::Blueprint::Code &code) {
  for (const auto &[key, value] : code.defines) {
    builder.addDefine(key, value);
  }
  builder
    .replace(kUserModulesRegion,
             makeCodePatch(FileIndex::UserModule, code.includes))
    .replace(kUserCodeRegion, makeCodePatch(FileIndex::Main, code.source));
}

} // namespace

void noMaterial(ShaderCodeBuilder &builder) {
  builder.addDefine("NO_MATERIAL", 1)
    .replace(kUserPropertiesRegion, "/* no properties */")
    .replace(kUserSamplersRegion, "/* no samplers */")
    .replace(kUserModulesRegion, "/* no user modules */")
    .replace(kUserCodeRegion, "/* no user code */");
}

void setReferenceFrames(ShaderCodeBuilder &builder, ReferenceFrames in) {
  using enum ReferenceFrame;

  switch (in.position) {
  case View:
    builder.addDefine("OUT_VERTEX_VIEW_SPACE", 1);
    break;
  case Clip:
    builder.addDefine("OUT_VERTEX_CLIP_SPACE", 1);
    break;

  default:
    assert(false);
  }

  switch (in.normal) {
  case World:
    builder.addDefine("OUT_NORMAL_WORLD_SPACE", 1);
    break;
  case View:
    builder.addDefine("OUT_NORMAL_VIEW_SPACE", 1);
    break;
  case Clip:
    builder.addDefine("OUT_NORMAL_CLIP_SPACE", 1);
    break;

  default:
    assert(false);
  }
}

void addMaterial(ShaderCodeBuilder &builder, const Material &material,
                 rhi::ShaderType shaderType, VkDeviceSize minOffsetAlignment) {
  const auto &blueprint = material.getBlueprint();

  if (isSurface(material)) addSurface(builder, *blueprint.surface);
  addProperties(builder, material, minOffsetAlignment);
  addSamplers(builder, blueprint.defaultTextures);

  if (shaderType == rhi::ShaderType::Vertex) {
    setReferenceFrames(builder, {
                                  .position = ReferenceFrame::Clip,
                                  .normal = ReferenceFrame::World,
                                });
  }

  const auto it = blueprint.userCode.find(shaderType);
  addCode(builder, it != blueprint.userCode.cend()
                     ? it->second
                     : Material::Blueprint::Code{});
}

} // namespace gfx
