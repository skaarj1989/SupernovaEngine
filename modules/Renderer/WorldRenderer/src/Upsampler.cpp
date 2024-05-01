#include "renderer/Upsampler.hpp"
#include "rhi/CommandBuffer.hpp"

#include "renderer/PostProcess.hpp"

#include "fg/FrameGraph.hpp"
#include "FrameGraphResourceAccess.hpp"
#include "renderer/FrameGraphTexture.hpp"

#include "RenderContext.hpp"

namespace gfx {

namespace {

[[nodiscard]] constexpr auto stepUp(const rhi::Extent2D e) {
  return rhi::Extent2D{.width = e.width * 2u, .height = e.height * 2u};
}

} // namespace

//
// Upsampler class:
//

Upsampler::Upsampler(rhi::RenderDevice &rd) : rhi::RenderPass<Upsampler>{rd} {}

FrameGraphResource Upsampler::addPass(FrameGraph &fg,
                                      const FrameGraphResource input,
                                      const float radius) {
  static constexpr auto kPassName = "UpsamplePass";
  ZoneScopedN(kPassName);

  struct Data {
    FrameGraphResource upsampled;
  };
  const auto [upsampled] = fg.addCallbackPass<Data>(
    kPassName,
    [&fg, input](FrameGraph::Builder &builder, Data &data) {
      PASS_SETUP_ZONE;

      builder.read(input, TextureRead{
                            .binding =
                              {
                                .location = {.set = 0, .binding = 0},
                                .pipelineStage = PipelineStage::FragmentShader,
                              },
                            .type = TextureRead::Type::CombinedImageSampler,
                            .imageAspect = rhi::ImageAspect::Color,
                          });

      const auto &inputDesc = fg.getDescriptor<FrameGraphTexture>(input);
      data.upsampled = builder.create<FrameGraphTexture>(
        "Upsampled", {
                       .extent = stepUp(inputDesc.extent),
                       .format = inputDesc.format,
                       .usageFlags = rhi::ImageUsage::RenderTarget |
                                     rhi::ImageUsage::Sampled,
                     });
      data.upsampled =
        builder.write(data.upsampled, Attachment{
                                        .index = 0,
                                        .imageAspect = rhi::ImageAspect::Color,
                                      });
    },
    [this, radius](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, commonSamplers, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      const auto *pipeline =
        _getPipeline(rhi::getColorFormat(*framebufferInfo, 0));
      if (pipeline) {
        overrideSampler(sets[0][0], commonSamplers.bilinear);

        cb.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);
        cb.pushConstants(rhi::ShaderStages::Fragment, 0, &radius)
          .beginRendering(*framebufferInfo)
          .drawFullScreenTriangle();
        endRendering(rc);
      }
    });

  return upsampled;
}

//
// (private):
//

rhi::GraphicsPipeline
Upsampler::_createPipeline(const rhi::PixelFormat colorFormat) const {
  return createPostProcessPipelineFromFile(getRenderDevice(), colorFormat,
                                           "Upsample.frag");
}

} // namespace gfx
