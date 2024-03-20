#include "MaterialEditor/Nodes/SurfaceMaster.hpp"
#include "Utility.hpp"

namespace {

constexpr glm::vec3 kWhite{1.0f};
constexpr glm::vec3 kBlack{0.0f};

using namespace gfx;

bool always(const Material::Surface &) { return true; }

bool litOnly(const Material::Surface &surface) {
  return surface.shadingModel == ShadingModel::Lit;
}
bool transparentOnly(const Material::Surface &surface) {
  using enum BlendMode;
  return surface.blendMode == Transparent || surface.blendMode == Add ||
         surface.blendMode == Modulate;
}
bool transmissiveOnly(const Material::Surface &surface) {
  return litOnly(surface) && surface.blendMode == BlendMode::Opaque &&
         surface.lightingMode == LightingMode::Transmission;
}

} // namespace

const std::vector<SurfaceMasterNode::FieldInfo> SurfaceMasterNode::kFields{
  {"baseColor", ValueVariant{kWhite}, litOnly},
  {
    "opacity",
    ValueVariant{1.0f},
    [](const auto &surface) {
      return transparentOnly(surface) || transmissiveOnly(surface) ||
             surface.blendMode == BlendMode::Masked;
    },
  },
  {"ior", ValueVariant{1.5f}, transmissiveOnly},
  {"transmissionFactor", ValueVariant{0.0f}, transmissiveOnly},
  {"thickness", ValueVariant{0.0f}, transmissiveOnly},
  {"attenuationColor", ValueVariant{kWhite}, transmissiveOnly},
  {"attenuationDistance", ValueVariant{1.0f}, transmissiveOnly},
  {"normal", Attribute::Normal, litOnly},
  {"metallic", ValueVariant{1.0f}, litOnly},
  {"roughness", ValueVariant{1.0f}, litOnly},
  {"specular", ValueVariant{1.0f}, litOnly},
  {"emissiveColor", ValueVariant{kBlack}, always},
  {
    "ambientOcclusion",
    ValueVariant{1.0f},
    [](const auto &surface) {
      using enum BlendMode;
      return litOnly(surface) &&
             (surface.blendMode == Opaque || surface.blendMode == Masked);
    },
  },
};

//
// SurfaceMasterNode class:
//

SurfaceMasterNode::SurfaceMasterNode(ShaderGraph &g, const IDPair vertex)
    : CompoundNode{g, vertex, Flags::None} {
  auto hint = vertex.id + 1;
  inputs.reserve(kFields.size());
  for (const auto &[name, value, _] : kFields) {
    auto *node = createNode(g, hint++, value);
    node->label = name;
    node->flags = Flags::Internal | Flags::Input;
    inputs.emplace_back(node->vertex);
  }
}

std::unique_ptr<NodeBase> SurfaceMasterNode::clone(const IDPair) const {
  throw std::logic_error("Forbidden operation.");
}

std::string SurfaceMasterNode::toString() const { return "SurfaceMaster"; }
