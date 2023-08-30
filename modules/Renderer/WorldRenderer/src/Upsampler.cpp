#include "renderer/Upsampler.hpp"

#include "fg/FrameGraph.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "renderer/CommonSamplers.hpp"
#include "renderer/PostProcess.hpp"
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

Upsampler::Upsampler(rhi::RenderDevice &rd,
                     const CommonSamplers &commonSamplers)
    : rhi::RenderPass<Upsampler>{rd}, m_samplers{commonSamplers} {}

FrameGraphResource Upsampler::addPass(FrameGraph &fg, FrameGraphResource input,
                                      float radius) {
  ZoneScoped;

  constexpr auto kPassName = "UpsamplePass";

  struct Data {
    FrameGraphResource upsampled;
  };
  const auto [upsampled] = fg.addCallbackPass<Data>(
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
      data.upsampled = builder.create<FrameGraphTexture>(
        "Upsampled", {
                       .extent = stepUp(inputDesc.extent),
                       .format = inputDesc.format,
                       .usageFlags = rhi::ImageUsage::RenderTarget |
                                     rhi::ImageUsage::Sampled,
                     });
      data.upsampled = builder.write(data.upsampled, Attachment{.index = 0});
    },
    [this, radius](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, kPassName)

      auto &[cb, framebufferInfo, sets] = rc;
      const auto *pipeline =
        _getPipeline(rhi::getColorFormat(*framebufferInfo, 0));
      if (pipeline) {
        overrideSampler(sets[0][0], m_samplers.bilinear);

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
Upsampler::_createPipeline(rhi::PixelFormat colorFormat) const {
  return createPostProcessPipelineFromFile(getRenderDevice(), colorFormat,
                                           "Upsample.frag");
}

} // namespace gfx
