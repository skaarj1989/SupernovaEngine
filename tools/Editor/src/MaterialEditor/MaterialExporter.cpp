#include "MaterialExporter.hpp"
#include "rhi/json.hpp"
#include "renderer/jsonMaterial.hpp"
#include <fstream> // ofstream

using namespace nlohmann;

struct TextureMeta {
  rhi::TextureType type{rhi::TextureType::Undefined};
  std::string path; // Relative to a material root file.
};
static void to_json(ordered_json &j, const TextureMeta &v) {
  j = ordered_json{{"type", v.type}, {"path", v.path}};
}

namespace {

[[nodiscard]] auto jsonify(const std::vector<gfx::Property> &in) {
  auto j = nlohmann::ordered_json::object();
  for (const auto &[key, value] : in) {
    j[key] = value;
  }
  return j;
}
[[nodiscard]] auto jsonify(const gfx::TextureResources &in,
                           const std::filesystem::path &p) {
  auto j = nlohmann::ordered_json::object();
  for (const auto &[key, value] : in) {
    if (auto r = std::dynamic_pointer_cast<Resource>(value.texture); r) {
      j[key] = TextureMeta{
        .type = value.texture->getType(),
        .path = r->getPath().lexically_relative(p).string(),
      };
    }
  }
  return j;
}

struct ShaderCodeChunk {
  std::vector<std::string> defines;
  std::optional<std::filesystem::path> include;
  std::optional<std::filesystem::path> code;
};

[[nodiscard]] auto saveUserCode(const std::filesystem::path &dir,
                                const gfx::Material::Blueprint::Code &code,
                                const std::string_view type) {
  ShaderCodeChunk result{.defines = code.defines};
  if (!code.source.empty()) {
    const auto p = dir / std::format("code.{}", type);
    if (std::ofstream f{p, std::ios::binary}; f.is_open()) {
      f << code.source;
      result.code = p.filename();
    }
  }
  if (!code.includes.empty()) {
    const auto p = dir / std::format("{}.module.glsl", type);
    if (std::ofstream f{p, std::ios::binary}; f.is_open()) {
      f << code.includes;
      result.include = p.filename();
    }
  }
  return result;
}

[[nodiscard]] auto jsonify(const ShaderCodeChunk &in) {
  auto j = nlohmann::ordered_json{{"defines", in.defines}};
  if (in.include) j["includes"] = {*in.include};
  if (in.code) j["path"] = *in.code;
  return j;
}

} // namespace

bool exportMaterial(const std::filesystem::path &p, const std::string_view name,
                    const gfx::Material::Blueprint &blueprint) {
  const auto materialDomain = getDomain(blueprint);

  auto j = ordered_json{
    {"name", name},
    {"domain", materialDomain},
  };
  if (materialDomain == gfx::MaterialDomain::Surface) {
    const auto &surface = *blueprint.surface;
    j["shadingModel"] = surface.shadingModel;
    j["blendMode"] = surface.blendMode;
    j["decalBlendMode"] = surface.decalBlendMode;
    j["lightingMode"] = surface.lightingMode;
    j["cullMode"] = surface.cullMode;
  }

  j["properties"] = jsonify(blueprint.properties);
  const auto rootDirectory = p.parent_path();
  j["samplers"] = jsonify(blueprint.defaultTextures, rootDirectory);

  const auto vert = saveUserCode(rootDirectory, blueprint.userVertCode, "vert");
  if (vert.code) j["vertexShader"] = jsonify(vert);
  const auto frag = saveUserCode(rootDirectory, blueprint.userFragCode, "frag");
  if (frag.code) j["fragmentShader"] = jsonify(frag);

  j["flags"] = getValues(blueprint.flags);

  if (std::ofstream f{p}; f.is_open()) {
    f << std::setw(2) << j << std::endl;
    return true;
  }
  return false;
}
