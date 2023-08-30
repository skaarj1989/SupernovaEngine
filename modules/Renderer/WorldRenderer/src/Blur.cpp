#include "renderer/Blur.hpp"

#include "fg/FrameGraph.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "renderer/CommonSamplers.hpp"

#include "renderer/PostProcess.hpp"
#include "RenderContext.hpp"

namespace gfx {

Blur::Blur(rhi::RenderDevice &rd, const CommonSamplers &commonSamplers)
    : rhi::RenderPass<Blur>{rd}, m_samplers{commonSamplers} {
  // Warmup ...
  _getPipeline(rhi::PixelFormat::R8_UNorm);    // For SSAO.
  _getPipeline(rhi::PixelFormat::RGBA8_UNorm); // Everything else.
}

uint32_t Blur::count(PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void Blur::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource Blur::addGaussianBlur(FrameGraph &fg,
                                         FrameGraphResource input,
                                         int32_t iterations, float scale) {
  ZoneScoped;

  for (auto i = 0; i < iterations; ++i) {
    const auto radius = float(iterations - i - 1) * scale;
    input = addTwoPassGaussianBlur(fg, input, radius);
  }
  return input;
}
FrameGraphResource Blur::addTwoPassGaussianBlur(FrameGraph &fg,
                                                FrameGraphResource input,
                                                float radius) {
  ZoneScoped;

  input = _addPass(fg, input, {radius, 0.0f});
  return _addPass(fg, input, {0.0f, radius});
}

//
// (private):
//

rhi::GraphicsPipeline
Blur::_createPipeline(rhi::PixelFormat colorFormat) const {
  return createPostProcessPipelineFromFile(getRenderDevice(), colorFormat,
                                           "Blur.frag");
}

FrameGraphResource Blur::_addPass(FrameGraph &fg, FrameGraphResource input,
                                  glm::vec2 direction) {
  ZoneScoped;

  const auto passName =
    std::format("Blur [x={}, y={}]", direction.x, direction.y);

  struct Data {
    FrameGraphResource output;
  };
  const auto [blurred] = fg.addCallbackPass<Data>(
    passName,
    [&fg, input](FrameGraph::Builder &builder, Data &data) {
      builder.read(input, TextureRead{
                            .binding =
                              {
                                .location = {.set = 0, .binding = 1},
                                .pipelineStage = PipelineStage::FragmentShader,
                              },
                            .type = TextureRead::Type::CombinedImageSampler,
                          });

      const auto &inputDesc = fg.getDescriptor<FrameGraphTexture>(input);
      data.output = builder.create<FrameGraphTexture>(
        "Blurred", {
                     .extent = inputDesc.extent,
                     .format = inputDesc.format,
                     .usageFlags =
                       rhi::ImageUsage::RenderTarget | rhi::ImageUsage::Sampled,
                   });
      data.output = builder.write(data.output, Attachment{.index = 0});
    },
    [this, passName, direction](const Data &, const FrameGraphPassResources &,
                                void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, passName.c_str())

      auto &[cb, framebufferInfo, sets] = rc;
      const auto *pipeline =
        _getPipeline(rhi::getColorFormat(*framebufferInfo, 0));
      if (pipeline) {
        overrideSampler(sets[0][1], m_samplers.bilinear);

        cb.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);
        cb.pushConstants(rhi::ShaderStages::Fragment, 0, &direction);
        renderFullScreenPostProcess(rc, *pipeline);
      }
    });

  return blurred;
}

} // namespace gfx
