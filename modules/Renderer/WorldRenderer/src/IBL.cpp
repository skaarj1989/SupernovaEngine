#include "renderer/IBL.hpp"
#include "rhi/RenderDevice.hpp"
#include "ShaderCodeBuilder.hpp"

#include "glm/common.hpp" // max

#if defined(_MSC_VER)
#  pragma warning(disable : 26812) // Silence unscoped enum
#endif

namespace gfx {

const auto kLodBias = 0.0f;

enum class Distribution : uint32_t { Lambertian = 0, GGX, Charlie };

//
// IBL class:
//

IBL::IBL(rhi::RenderDevice &rd) : m_renderDevice{rd} {
  m_brdfPipeline = rd.createComputePipeline(
    ShaderCodeBuilder{}.buildFromFile("Utility/GenerateBRDF.comp"));
}

uint32_t IBL::count(const PipelineGroups flags) const {
  uint32_t n{0};
  if (bool(flags & PipelineGroups::BuiltIn)) {
    n += m_irradianceGenerator.count() + m_prefilterGenerator.count() + 1;
  }
  return n;
}
void IBL::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) {
    m_irradianceGenerator.clear();
    m_prefilterGenerator.clear();
  }
}

rhi::Texture IBL::generateBRDF(rhi::CommandBuffer &cb) {
  RHI_GPU_ZONE(cb, "GenerateBRDF");

  constexpr auto kSize = 1024u;

  auto brdf =
    rhi::Texture::Builder{}
      .setExtent({kSize, kSize})
      .setPixelFormat(rhi::PixelFormat::RG16F)
      .setNumMipLevels(1)
      .setUsageFlags(rhi::ImageUsage::Storage | rhi::ImageUsage::Sampled)
      .build(m_renderDevice);

  m_renderDevice.setupSampler(
    brdf, {
            .magFilter = rhi::TexelFilter::Linear,
            .minFilter = rhi::TexelFilter::Linear,
            .mipmapMode = rhi::MipmapMode::Nearest,

            .addressModeS = rhi::SamplerAddressMode::ClampToEdge,
            .addressModeT = rhi::SamplerAddressMode::ClampToEdge,
          });

  // ---

  cb.getBarrierBuilder().imageBarrier(
    {
      .image = brdf,
      .newLayout = rhi::ImageLayout::General,
      .subresourceRange = {.levelCount = 1, .layerCount = 1},
    },
    {
      .stageMask = rhi::PipelineStages::ComputeShader,
      .accessMask = rhi::Access::ShaderWrite,
    });

  // -- [set = 0]

  constexpr auto kDescriptorSetId = 0;

  const auto descriptors =
    cb.createDescriptorSetBuilder()
      .bind(0, rhi::bindings::StorageImage{&brdf, 0})
      .build(m_brdfPipeline.getDescriptorSetLayout(kDescriptorSetId));

  // ---

  constexpr auto kLocalSize = 32u;
  const auto kDistribution = Distribution::GGX;

  cb.bindPipeline(m_brdfPipeline)
    .bindDescriptorSet(kDescriptorSetId, descriptors)
    .pushConstants(rhi::ShaderStages::Compute, 0, &kDistribution)
    .dispatch({glm::uvec2{kSize} / kLocalSize, 1u});

  cb.getBarrierBuilder().imageBarrier(
    {
      .image = brdf,
      .newLayout = rhi::ImageLayout::ReadOnly,
      .subresourceRange = {.levelCount = 1, .layerCount = 1},
    },
    {
      .stageMask = rhi::PipelineStages::FragmentShader,
      .accessMask = rhi::Access::ShaderRead,
    });

  return brdf;
}

//
// IrradianceGenerator class:
//

IBL::IrradianceGenerator::IrradianceGenerator(rhi::RenderDevice &rd)
    : rhi::ComputePass<IrradianceGenerator>{rd} {}

rhi::Texture IBL::IrradianceGenerator::generate(rhi::CommandBuffer &cb,
                                                const rhi::Texture &source) {
  assert(source);
  RHI_GPU_ZONE(cb, "GenerateIrradiance");

  auto &rd = getRenderDevice();

  constexpr auto kSize = 64u;
  auto irradiance =
    rhi::Texture::Builder{}
      .setExtent({kSize, kSize})
      .setPixelFormat(source.getPixelFormat())
      .setNumMipLevels(1)
      .setCubemap(true)
      .setUsageFlags(rhi::ImageUsage::Transfer | rhi::ImageUsage::Storage |
                     rhi::ImageUsage::Sampled)
      .build(rd);

  rd.setupSampler(irradiance,
                  {
                    .magFilter = rhi::TexelFilter::Linear,
                    .minFilter = rhi::TexelFilter::Linear,
                    .mipmapMode = rhi::MipmapMode::Nearest,

                    .addressModeS = rhi::SamplerAddressMode::ClampToEdge,
                    .addressModeT = rhi::SamplerAddressMode::ClampToEdge,
                    .addressModeR = rhi::SamplerAddressMode::ClampToEdge,
                  });

  // ---

  cb.getBarrierBuilder().imageBarrier(
    {
      .image = irradiance,
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

  const auto *pipeline = _getPipeline();
  if (!pipeline) return {};

  // -- [set = 0]

  constexpr auto kDescriptorSetId = 0;

  const auto descriptors =
    cb.createDescriptorSetBuilder()
      .bind(0, rhi::bindings::CombinedImageSampler{&source})
      .bind(1, rhi::bindings::StorageImage{&irradiance, 0})
      .build(pipeline->getDescriptorSetLayout(kDescriptorSetId));

  // ---

  constexpr auto kLocalSize = 8u;

  cb.bindPipeline(*pipeline)
    .bindDescriptorSet(kDescriptorSetId, descriptors)
    .pushConstants(rhi::ShaderStages::Compute, 0, &kLodBias)
    .dispatch({glm::uvec2{kSize / kLocalSize}, 1u});

  cb.getBarrierBuilder().imageBarrier(
    {
      .image = irradiance,
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

  return irradiance;
}

rhi::ComputePipeline IBL::IrradianceGenerator::_createPipeline() const {
  return getRenderDevice().createComputePipeline(
    ShaderCodeBuilder{}.buildFromFile("Utility/BakeIrradiance.comp"));
}

//
// PrefilterGenerator class:
//

IBL::PrefilterGenerator::PrefilterGenerator(rhi::RenderDevice &rd)
    : rhi::ComputePass<PrefilterGenerator>{rd} {}

rhi::Texture IBL::PrefilterGenerator::generate(rhi::CommandBuffer &cb,
                                               const rhi::Texture &source) {
  assert(source);
  RHI_GPU_ZONE(cb, "PrefilterEnvMap");

  auto &rd = getRenderDevice();

  const auto size = source.getExtent().width;
  auto prefilteredEnvMap =
    rhi::Texture::Builder{}
      .setExtent({size, size})
      .setPixelFormat(source.getPixelFormat())
      .setCubemap(true)
      .setUsageFlags(rhi::ImageUsage::Storage | rhi::ImageUsage::Sampled)
      .build(rd);

  const auto numMipLevels = prefilteredEnvMap.getNumMipLevels();
  rd.setupSampler(prefilteredEnvMap,
                  {
                    .magFilter = rhi::TexelFilter::Linear,
                    .minFilter = rhi::TexelFilter::Linear,
                    .mipmapMode = rhi::MipmapMode::Linear,

                    .addressModeS = rhi::SamplerAddressMode::ClampToEdge,
                    .addressModeT = rhi::SamplerAddressMode::ClampToEdge,
                    .addressModeR = rhi::SamplerAddressMode::ClampToEdge,

                    .maxLod = float(numMipLevels),
                  });

  // ---

  cb.getBarrierBuilder().imageBarrier(
    {
      .image = prefilteredEnvMap,
      .newLayout = rhi::ImageLayout::General,
      .subresourceRange =
        {
          .levelCount = VK_REMAINING_MIP_LEVELS,
          .layerCount = VK_REMAINING_ARRAY_LAYERS,
        },
    },
    {
      .stageMask = rhi::PipelineStages::ComputeShader,
      .accessMask = rhi::Access::ShaderWrite,
    });

  const auto *pipeline = _getPipeline(numMipLevels);
  if (!pipeline) return {};

  // -- [set = 0]

  constexpr auto kDescriptorSetId = 0;

  const auto descriptors =
    cb.createDescriptorSetBuilder()
      .bind(0, rhi::bindings::CombinedImageSampler{&source})
      .bind(1, rhi::bindings::StorageImage{&prefilteredEnvMap})
      .build(pipeline->getDescriptorSetLayout(kDescriptorSetId));

  // ---

  constexpr auto kLocalSize = 8u;

  cb.bindPipeline(*pipeline).bindDescriptorSet(kDescriptorSetId, descriptors);

  for (auto level = 0u; level < numMipLevels; ++level) {
    struct Uniforms {
      uint32_t mipLevel;
      float roughness;
      float lodBias{kLodBias};
    };
    const Uniforms uniforms{
      .mipLevel = level,
      .roughness = float(level) / (numMipLevels - 1u),
    };

    const auto mipSize = rhi::calcMipSize(glm::uvec3{size, size, 1u}, level).x;
    const auto numGroups = glm::max(1u, mipSize / kLocalSize);

    cb.pushConstants(rhi::ShaderStages::Compute, 0, &uniforms)
      .dispatch({glm::uvec2{numGroups}, 1u});
  }

  cb.getBarrierBuilder().imageBarrier(
    {
      .image = prefilteredEnvMap,
      .newLayout = rhi::ImageLayout::ReadOnly,
    },
    {
      .stageMask = rhi::PipelineStages::FragmentShader |
                   rhi::PipelineStages::ComputeShader,
      .accessMask = rhi::Access::ShaderRead,
    });

  return prefilteredEnvMap;
}

rhi::ComputePipeline
IBL::PrefilterGenerator::_createPipeline(const uint32_t numMipLevels) const {
  return getRenderDevice().createComputePipeline(
    ShaderCodeBuilder{}
      .addDefine("NUM_MIP_LEVELS", numMipLevels)
      .buildFromFile("Utility/PrefilterEnvMap.comp"));
}

} // namespace gfx
