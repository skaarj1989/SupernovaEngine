#include "UploadLights.hpp"

#include "fg/FrameGraph.hpp"
#include "renderer/FrameGraphBuffer.hpp"
#include "FrameGraphResourceAccess.hpp"
#include "fg/Blackboard.hpp"

#include "FrameGraphData/Lights.hpp"

#include "RenderContext.hpp"

#include "glm/geometric.hpp"     // normalize
#include "glm/trigonometric.hpp" // radians

namespace gfx {

namespace {

constexpr auto kNoShadowMap = ~0;

// see _LightBuffer in shaders/Lib/Light.glsl
constexpr auto kLightDataOffset = sizeof(uint32_t) * 4;

// shaders/Lib/Light.glsl
// - Position and direction in world-space.
// - Angles in radians.
struct alignas(16) GPULight {
  GPULight(const Light &light, int32_t index)
      : position{light.position, glm::max(0.0f, light.range)},
        color{light.color, glm::max(0.0f, light.intensity)},
        type{uint32_t(light.type)}, shadowMapIndex{index} {
    if (light.type != LightType::Point) {
      direction = glm::vec4{glm::normalize(light.direction), 0.0f};
    }
    direction.w = light.shadowBias;

    if (light.type == LightType::Spot) {
      innerConeAngle = glm::radians(light.innerConeAngle);
      outerConeAngle = glm::radians(light.outerConeAngle);
    }
  }

  glm::vec4 position{0.0f, 0.0f, 0.0f, 1.0f};  // .w = range
  glm::vec4 direction{0.0f, 0.0f, 0.0f, 0.0f}; // .w = shadowBias
  glm::vec4 color{0.0f, 0.0f, 0.0f, 1.0f};     // .a = intensity
  uint32_t type{0};
  int32_t shadowMapIndex{kNoShadowMap};
  float innerConeAngle{1.0f};
  float outerConeAngle{1.0f};
};
static_assert(sizeof(GPULight) == 64);

[[nodiscard]] std::optional<int32_t>
findShadowMapIndex(const Light &light, const ShadowMapIndices &indices) {
  if (auto pairsIt = indices.find(light.type); pairsIt != indices.cend()) {
    auto it = std::ranges::find_if(
      pairsIt->second, [&](const auto &p) { return p.first == &light; });
    if (it != pairsIt->second.cend()) return it->second;
  }
  return std::nullopt;
}

[[nodiscard]] auto convert(std::span<const Light *const> lights,
                           const ShadowMapIndices &indices) {
  std::vector<GPULight> gpuLights;
  gpuLights.reserve(lights.size());

  std::ranges::transform(
    lights, std::back_inserter(gpuLights), [&](const Light *light) {
      const auto shadowMapIndex = findShadowMapIndex(*light, indices);
      return GPULight{*light, shadowMapIndex.value_or(kNoShadowMap)};
    });

  return gpuLights;
}

} // namespace

void uploadLights(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                  std::vector<const Light *> &&visibleLights,
                  ShadowMapIndices &&indices) {
  constexpr auto kPassName = "UploadLights";
  ZoneScopedN(kPassName);

  const auto numLights = uint32_t(visibleLights.size());

  blackboard.add<LightsData>() = fg.addCallbackPass<LightsData>(
    kPassName,
    [numLights](FrameGraph::Builder &builder, LightsData &data) {
      PASS_SETUP_ZONE;

      constexpr auto kMinNumLights = 1024u;
      const auto kBufferSize =
        kLightDataOffset +
        (sizeof(GPULight) * std::max(numLights, kMinNumLights));
      data.lights = builder.create<FrameGraphBuffer>(
        "LightsBuffer", {
                          .type = BufferType::StorageBuffer,
                          .stride = sizeof(std::byte),
                          .capacity = kBufferSize,
                        });
      data.lights = builder.write(
        data.lights, BindingInfo{.pipelineStage = PipelineStage::Transfer});
    },
    [visibleLights = std::move(visibleLights), indices = std::move(indices),
     numLights](const LightsData &data, FrameGraphPassResources &resources,
                void *ctx) {
      auto &cb = static_cast<RenderContext *>(ctx)->commandBuffer;
      RHI_GPU_ZONE(cb, kPassName);

      auto &buffer = *resources.get<FrameGraphBuffer>(data.lights).buffer;

      cb.update(buffer, 0, sizeof(uint32_t), &numLights);
      if (numLights > 0) {
        const auto gpuLights = convert(visibleLights, indices);
        cb.update(buffer, kLightDataOffset, sizeof(GPULight) * numLights,
                  gpuLights.data());
      }
    });
}

} // namespace gfx
