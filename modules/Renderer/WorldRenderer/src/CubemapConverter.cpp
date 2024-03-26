#include "renderer/CubemapConverter.hpp"
#include "math/Math.hpp"
#include "rhi/RenderDevice.hpp"
#include "ShaderCodeBuilder.hpp"
#include "glm/gtc/constants.hpp"

namespace gfx {

namespace {

constexpr auto kTileSize = 8u;

[[nodiscard]] uint32_t calculateCubeSize(const rhi::Texture &equirectangular) {
  const auto h = equirectangular.getExtent().height;
  const auto x = 8.0f * (float(glm::floor(2u * h) / glm::pi<float>() / 8.0f));
  // or: width / glm::pi<float>();
  // or: (glm::sqrt(3) / 6) * width;

  return nextPowerOf2(uint32_t(x));
}

} // namespace

//
// CubemapConverter class:
//

CubemapConverter::CubemapConverter(rhi::RenderDevice &rd)
    : rhi::ComputePass<CubemapConverter>{rd} {}

uint32_t CubemapConverter::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void CubemapConverter::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

rhi::Texture CubemapConverter::equirectangularToCubemap(
  rhi::CommandBuffer &cb, const rhi::Texture &equirectangular) {
  assert(equirectangular &&
         equirectangular.getType() == rhi::TextureType::Texture2D);
  RHI_GPU_ZONE(cb, "Equirectangular -> Cubemap");

  auto &rd = getRenderDevice();

  const auto size = calculateCubeSize(equirectangular);
  auto cubemap =
    rhi::Texture::Builder{}
      .setExtent({size})
      .setPixelFormat(rhi::PixelFormat::RGBA16F)
      .setCubemap(true)
      .setUsageFlags(rhi::ImageUsage::Transfer | rhi::ImageUsage::Storage |
                     rhi::ImageUsage::Sampled)
      .setupOptimalSampler(false)
      .build(rd);

  rd.setupSampler(cubemap,
                  {
                    .magFilter = rhi::TexelFilter::Linear,
                    .minFilter = rhi::TexelFilter::Linear,
                    .mipmapMode = rhi::MipmapMode::Linear,

                    .addressModeS = rhi::SamplerAddressMode::ClampToEdge,
                    .addressModeT = rhi::SamplerAddressMode::ClampToEdge,
                    .addressModeR = rhi::SamplerAddressMode::ClampToEdge,

                    .maxLod = float(cubemap.getNumMipLevels()),
                  });

  // ---

  cb.getBarrierBuilder().imageBarrier(
    {
      .image = cubemap,
      .newLayout = rhi::ImageLayout::General,
      .subresourceRange =
        {
          .levelCount = 1,
          .layerCount = VK_REMAINING_ARRAY_LAYERS,
        },
    },
    {
      .stageMask = rhi::PipelineStages::ComputeShader,
      .accessMask = rhi::Access::ShaderWrite,
    });

  const auto *pipeline = _getPipeline(cubemap.getPixelFormat());
  if (!pipeline) return {};

  // -- [set = 0]

  constexpr auto kDescriptorSetId = 0;

  auto descriptors =
    cb.createDescriptorSetBuilder()
      .bind(0, rhi::bindings::CombinedImageSampler{&equirectangular})
      .bind(1, rhi::bindings::StorageImage{&cubemap, 0})
      .build(pipeline->getDescriptorSetLayout(kDescriptorSetId));

  // ---

  cb.bindPipeline(*pipeline)
    .bindDescriptorSet(kDescriptorSetId, descriptors)
    .dispatch({glm::uvec2{size / kTileSize}, 1u});

  cb.generateMipmaps(cubemap);

  cb.getBarrierBuilder().imageBarrier(
    {
      .image = cubemap,
      .newLayout = rhi::ImageLayout::General,
      .subresourceRange =
        {
          .levelCount = VK_REMAINING_MIP_LEVELS,
          .layerCount = VK_REMAINING_ARRAY_LAYERS,
        },
    },
    {
      .stageMask = rhi::PipelineStages::FragmentShader |
                   rhi::PipelineStages::ComputeShader,
      .accessMask = rhi::Access::ShaderRead,
    });

  return cubemap;
}

//
// (private):
//

rhi::ComputePipeline
CubemapConverter::_createPipeline(const rhi::PixelFormat) const {
  return getRenderDevice().createComputePipeline(
    ShaderCodeBuilder{}
      .addDefine("TILE_SIZE", kTileSize)
      .buildFromFile("Utility/EquirectangularToCubemap.comp"));
}

} // namespace gfx
