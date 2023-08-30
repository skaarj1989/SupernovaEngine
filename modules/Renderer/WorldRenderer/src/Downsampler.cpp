#include "renderer/Downsampler.hpp"

#include "fg/FrameGraph.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "renderer/CommonSamplers.hpp"

#include "renderer/PostProcess.hpp"
#include "RenderContext.hpp"

#include "glm/common.hpp" // max

namespace gfx {

namespace {

[[nodiscard]] constexpr auto stepDown(const rhi::Extent2D e) {
  return rhi::Extent2D{
    .width = glm::max(uint32_t(float(e.width) * 0.5f), 1u),
    .height = glm::max(uint32_t(float(e.height) * 0.5f), 1u),
  };
}

} // namespace

Downsampler::Downsampler(rhi::RenderDevice &rd,
                         const CommonSamplers &commonSamplers)
    : rhi::RenderPass<Downsampler>{rd}, m_samplers{commonSamplers} {}

FrameGraphResource
Downsampler::addPass(FrameGraph &fg, FrameGraphResource input, uint32_t level) {
  ZoneScoped;

  constexpr auto kPassName = "DownsamplePass";

  struct Data {
    FrameGraphResource downsampled;
  };
  const auto [downsampled] = fg.addCallbackPass<Data>(
    kPassName,
    [&fg, input](FrameGraph::Builder &builder, Data &data) {
      builder.read(input, TextureRead{
                            .binding =
                              {
                                .location = {.set = 0, .binding = 0},
                                .pipelineStage = PipelineStage::FragmentShader,
                              },
                            .type = TextureRead::Type::CombinedImageSampler,
                          });

      const auto &inputDesc = fg.getDescriptor<FrameGraphTexture>(input);
      data.downsampled = builder.create<FrameGraphTexture>(
        "Downsampled", {
                         .extent = stepDown(inputDesc.extent),
                         .format = inputDesc.format,
                         .usageFlags = rhi::ImageUsage::RenderTarget |
                                       rhi::ImageUsage::Sampled,
                       });
      data.downsampled = builder.write(
        data.downsampled, Attachment{
                            .index = 0,
                            .clearValue = ClearValue::TransparentBlack,
                          });
    },
    [this, level](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, kPassName)

      auto &[cb, framebufferInfo, sets] = rc;
      const auto *pipeline =
        _getPipeline(rhi::getColorFormat(*framebufferInfo, 0));
      if (pipeline) {
        overrideSampler(sets[0][0], m_samplers.bilinear);

        cb.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);
        cb.pushConstants(rhi::ShaderStages::Fragment, 0, &level)
          .beginRendering(*framebufferInfo)
          .drawFullScreenTriangle();
        endRendering(rc);
      }
    });

  return downsampled;
}

//
// (private):
//

rhi::GraphicsPipeline
Downsampler::_createPipeline(rhi::PixelFormat colorFormat) const {
  return createPostProcessPipelineFromFile(getRenderDevice(), colorFormat,
                                           "Downsample.frag");
}

} // namespace gfx
