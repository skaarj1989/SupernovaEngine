#include "MaterialEditor/Nodes/SurfaceMaster.hpp"
#include "VariantCast.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include <ranges>
#include <format>

namespace {

using DefaultValue = std::variant<ValueVariant, Attribute>;

[[nodiscard]] auto getDataType(const DefaultValue &v) {
  return std::visit([](const auto &arg) { return ::getDataType(arg); }, v);
}

bool always(const gfx::Material::Surface &) { return true; }

bool litOnly(const gfx::Material::Surface &surface) {
  return surface.shadingModel == gfx::ShadingModel::Lit;
}
bool transparentOnly(const gfx::Material::Surface &surface) {
  using enum gfx::BlendMode;
  return surface.blendMode == Transparent || surface.blendMode == Add ||
         surface.blendMode == Modulate;
}
bool transmissiveOnly(const gfx::Material::Surface &surface) {
  return litOnly(surface) && surface.blendMode == gfx::BlendMode::Opaque &&
         surface.lightingMode == gfx::LightingMode::Transmission;
}

constexpr glm::vec3 kWhite{1.0f};
constexpr glm::vec3 kBlack{0.0f};

struct FieldInfo {
  const char *name;
  DefaultValue defaultValue;

  bool (*isAvailable)(const gfx::Material::Surface &){always};
};
constexpr std::array<FieldInfo, 13> kSurfaceFields{
  FieldInfo{"baseColor", ValueVariant{kWhite}, litOnly},
  {
    "opacity",
    ValueVariant{1.0f},
    [](const auto &surface) {
      return transparentOnly(surface) || transmissiveOnly(surface) ||
             surface.blendMode == gfx::BlendMode::Masked;
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
  {"emissiveColor", ValueVariant{kBlack}},
  {
    "ambientOcclusion",
    ValueVariant{1.0f},
    [](const auto &surface) {
      using enum gfx::BlendMode;
      return litOnly(surface) &&
             (surface.blendMode == Opaque || surface.blendMode == Masked);
    },
  },
};

} // namespace

//
// SurfaceMasterNode struct:
//

SurfaceMasterNode SurfaceMasterNode::create(ShaderGraph &g,
                                            VertexDescriptor parent) {
  SurfaceMasterNode node{};
  node.inputs.reserve(kSurfaceFields.size());
  std::ranges::transform(
    kSurfaceFields, std::back_inserter(node.inputs), [&](const auto &info) {
      return createInternalInput(g, parent, info.name,
                                 variant_cast(info.defaultValue));
    });
  return node;
}

bool SurfaceMasterNode::inspect(ShaderGraph &g, int32_t,
                                const gfx::Material::Surface &surface) {
  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted("SurfaceMaster");
  ImNodes::EndNodeTitleBar();

  auto changed = false;
  for (auto [i, vd] : std::views::enumerate(inputs)) {
    const auto &[name, _, isAvailable] = kSurfaceFields[i];
    changed |=
      addInputPin(g, vd, {.name = name, .active = isAvailable(surface)},
                  InspectorMode::Popup, false);
    ++i;
  }

  return changed;
}
MasterNodeResult SurfaceMasterNode::evaluate(MaterialGenerationContext &context,
                                             int32_t id) const {
  auto &[_, tokens, composer] = *context.currentShader;

  assert(tokens.size() == kSurfaceFields.size());

  for (auto [i, arg] :
       extract<kSurfaceFields.size()>(tokens) | std::views::enumerate) {
    const auto &[name, defaultValue, _] = kSurfaceFields[i];
    const auto requiredType = getDataType(defaultValue);
    if (auto argStr = assure(arg, requiredType); argStr) {
      composer.addExpression(std::format("material.{} = {};", name, *argStr));
    } else {
      return std::unexpected{
        std::format("'{}': Can't convert {} -> {}.", name,
                    toString(arg.dataType), toString(requiredType)),
      };
    }
  }

  return std::monostate{};
}
