#include "renderer/SSAO.hpp"
#include "rhi/RenderDevice.hpp"

#include "renderer/CommonSamplers.hpp"
#include "renderer/PostProcess.hpp"
#include "renderer/Blur.hpp"

#include "renderer/SSAOSettings.hpp"

#include "FrameGraphResourceAccess.hpp"
#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphTexture.hpp"

#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/SSAO.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp" // normalize

#include <random>

namespace gfx {

namespace {

constexpr auto kKernelSize = 32;

[[nodiscard]] auto generateNoise(const uint32_t size) {
  std::uniform_real_distribution<float> dist{0.0, 1.0};
  std::random_device rd{};
  std::default_random_engine g{rd()};

  std::vector<glm::vec2> noise;
  noise.reserve(size);
  std::generate_n(std::back_inserter(noise), size, [&] {
    // Rotate around z-axis (in tangent space).
    return glm::vec2{
      dist(g) * 2.0f - 1.0f,
      dist(g) * 2.0f - 1.0f,
    };
  });
  return noise;
}
[[nodiscard]] auto generateNoiseTexture(rhi::RenderDevice &rd) {
  constexpr auto kSize = 4u;

  const auto noise = generateNoise(kSize * kSize);
  const auto dataSize = sizeof(decltype(noise)::value_type) * noise.size();

  auto stagingBuffer = rd.createStagingBuffer(dataSize, noise.data());

  auto texture =
    rhi::Texture::Builder{}
      .setExtent({kSize, kSize})
      .setPixelFormat(rhi::PixelFormat::RG32F)
      .setNumMipLevels(1)
      .setNumLayers(std::nullopt)
      .setUsageFlags(rhi::ImageUsage::TransferDst | rhi::ImageUsage::Sampled)
      .setupOptimalSampler(false)
      .build(rd);

  rd.execute([&stagingBuffer, &texture](rhi::CommandBuffer &cb) {
      cb.copyBuffer(stagingBuffer, texture);
      rhi::prepareForReading(cb, texture);
    })
    .setupSampler(texture, {
                             .magFilter = rhi::TexelFilter::Linear,
                             .minFilter = rhi::TexelFilter::Linear,
                             .mipmapMode = rhi::MipmapMode::Nearest,

                             .addressModeS = rhi::SamplerAddressMode::Repeat,
                             .addressModeT = rhi::SamplerAddressMode::Repeat,
                           });

  return texture;
}

[[nodiscard]] auto generateKernel(const uint32_t kernelSize) {
  std::uniform_real_distribution<float> dist{0.0f, 1.0f};
  std::random_device rd{};
  std::default_random_engine g{rd()};

  std::vector<glm::vec4> kernel;
  kernel.reserve(kernelSize);
  std::generate_n(std::back_inserter(kernel), kernelSize, [&, i = 0]() mutable {
    glm::vec3 sample{
      dist(g) * 2.0f - 1.0f,
      dist(g) * 2.0f - 1.0f,
      dist(g),
    };
    sample = glm::normalize(sample);
    sample *= dist(g);

    auto scale = float(i++) / float(kernelSize);
    scale = glm::mix(0.1f, 1.0f, scale * scale);
    return glm::vec4{sample * scale, 0.0f};
  });
  return kernel;
}
[[nodiscard]] auto generateSampleKernelBuffer(rhi::RenderDevice &rd) {
  const auto kernel = generateKernel(kKernelSize);
  const auto dataSize = sizeof(decltype(kernel)::value_type) * kernel.size();
  auto stagingBuffer = rd.createStagingBuffer(dataSize, kernel.data());
  auto uniformBuffer = rd.createUniformBuffer(dataSize);
  rd.execute([&](rhi::CommandBuffer &cb) {
    cb.copyBuffer(stagingBuffer, uniformBuffer, {.size = dataSize});
  });

  return uniformBuffer;
}

} // namespace

//
// SSAO class:
//

SSAO::SSAO(rhi::RenderDevice &rd, const CommonSamplers &commonSamplers)
    : rhi::RenderPass<SSAO>{rd}, m_samplers{commonSamplers} {
  m_noise = generateNoiseTexture(rd);
  m_kernelBuffer = generateSampleKernelBuffer(rd);
}

uint32_t SSAO::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void SSAO::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

void SSAO::addPass(FrameGraph &fg, FrameGraphBlackboard &blackboard, Blur &blur,
                   const Settings &settings) {
  static constexpr auto kPassName = "SSAO";
  ZoneScopedN(kPassName);

  const auto [ssao] = fg.addCallbackPass<SSAOData>(
    kPassName,
    [&fg, &blackboard](FrameGraph::Builder &builder, SSAOData &data) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<CameraData>(),
           PipelineStage::FragmentShader);

      const auto &gBuffer = blackboard.get<GBufferData>();
      read(builder, gBuffer, GBufferFlags::Depth | GBufferFlags::Normal);

      const auto inputExtent =
        fg.getDescriptor<FrameGraphTexture>(gBuffer.depth).extent;
      data.ssao = builder.create<FrameGraphTexture>(
        "SSAO", {
                  .extent = inputExtent,
                  .format = rhi::PixelFormat::R8_UNorm,
                  .usageFlags =
                    rhi::ImageUsage::RenderTarget | rhi::ImageUsage::Sampled,
                });
      data.ssao =
        builder.write(data.ssao, Attachment{
                                   .index = 0,
                                   .clearValue = ClearValue::OpaqueWhite,
                                 });
    },
    [this, settings](const SSAOData &, const FrameGraphPassResources &,
                     void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      const auto *pipeline =
        _getPipeline(rhi::getColorFormat(*framebufferInfo, 0));
      if (pipeline) {
        auto &bindings = sets[2];
        bindings[0] = rhi::bindings::SeparateSampler{m_samplers.bilinear};
        bindings[1] = rhi::bindings::UniformBuffer{.buffer = &m_kernelBuffer};
        bindings[2] = rhi::bindings::CombinedImageSampler{.texture = &m_noise};

        cb.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);
        cb.pushConstants(rhi::ShaderStages::Fragment, 0, &settings)
          .beginRendering(*framebufferInfo)
          .drawFullScreenTriangle();
        endRendering(rc);
      }
    });

  blackboard.add<SSAOData>().ssao = blur.addTwoPassGaussianBlur(fg, ssao, 1.0f);
}

//
// (private):
//

rhi::GraphicsPipeline
SSAO::_createPipeline(const rhi::PixelFormat colorFormat) const {
  return createPostProcessPipeline(getRenderDevice(), colorFormat,
                                   ShaderCodeBuilder{}
                                     .addDefine("KERNEL_SIZE", kKernelSize)
                                     .buildFromFile("SSAO.frag"));
}

} // namespace gfx
