#include "UploadShadowBlock.hpp"
#include "UploadStruct.hpp"
#include "BiasMatrix.hpp"

#include <ranges>

namespace gfx {

namespace {

static_assert(kMaxNumCascades <= 4);
struct GPUCascades {
  glm::vec4 splitDepth{};
  std::array<glm::mat4, kMaxNumCascades> viewProjMatrices{};
};
static_assert(sizeof(GPUCascades) == 272);

constexpr auto kMaxNumSpotLightShadows = 4;

// shaders/resources/ShadowBlock.glsl
struct GPUShadowBlock {
  explicit GPUShadowBlock(const ShadowBlock &shadowBlock) {
    for (auto [i, src] : std::views::enumerate(shadowBlock.cascades)) {
      cascades.splitDepth[i] = src.splitDepth;
      cascades.viewProjMatrices[i] =
        kBiasMatrix * src.lightView.viewProjection();
    }
    std::ranges::transform(shadowBlock.spotLightViewProjections,
                           spotLightViewProjections.begin(),
                           [](const auto &m) { return kBiasMatrix * m; });
  }

  GPUCascades cascades{};
  std::array<glm::mat4, kMaxNumSpotLightShadows> spotLightViewProjections{};
};
static_assert(sizeof(GPUShadowBlock) == 528);

} // namespace

FrameGraphResource uploadShadowBlock(FrameGraph &fg,
                                     const ShadowBlock &shadowBlock) {
  return uploadStruct(fg, "UploadShadowBlock",
                      TransientBuffer{
                        .name = "ShadowBlock",
                        .type = BufferType::UniformBuffer,
                        .data = GPUShadowBlock{shadowBlock},
                      });
}

} // namespace gfx
