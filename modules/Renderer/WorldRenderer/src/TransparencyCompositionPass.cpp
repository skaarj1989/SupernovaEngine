#include "renderer/TransparencyCompositionPass.hpp"
#include "rhi/CommandBuffer.hpp"

#include "fg/FrameGraph.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "FrameGraphData/WeightedBlended.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

namespace gfx {

TransparencyCompositionPass::TransparencyCompositionPass(rhi::RenderDevice &rd)
    : rhi::RenderPass<TransparencyCompositionPass>{rd} {}

FrameGraphResource
TransparencyCompositionPass::addPass(FrameGraph &fg,
                                     const WeightedBlendedData &weightedBlended,
                                     FrameGraphResource target) {
  constexpr auto kPassName = "TransparencyComposition";
  ZoneScopedN(kPassName);

  fg.addCallbackPass(
    kPassName,
    [&weightedBlended, &target](FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      builder.read(weightedBlended.accum,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 0, .binding = 1},
                         .pipelineStage = PipelineStage::FragmentShader,
                       },
                     .type = TextureRead::Type::SampledImage,
                   });
      builder.read(weightedBlended.reveal,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 0, .binding = 2},
                         .pipelineStage = PipelineStage::FragmentShader,
                       },
                     .type = TextureRead::Type::SampledImage,
                   });

      target = builder.write(target, Attachment{.index = 0});
    },
    [this](const auto &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      if (const auto *pipeline =
            _getPipeline(rhi::getColorFormat(*framebufferInfo, 0));
          pipeline) {
        renderFullScreenPostProcess(rc, *pipeline);
      }
    });

  return target;
}

//
// (private):
//

rhi::GraphicsPipeline TransparencyCompositionPass::_createPipeline(
  const rhi::PixelFormat colorFormat) const {
  ShaderCodeBuilder shaderCodeBuilder;

  return rhi::GraphicsPipeline::Builder{}
    .setColorFormats({colorFormat})
    .setInputAssembly({})
    .addShader(rhi::ShaderType::Vertex,
               shaderCodeBuilder.buildFromFile("FullScreenTriangle.vert"))
    .addShader(rhi::ShaderType::Fragment, shaderCodeBuilder.buildFromFile(
                                            "TransparencyCompositionPass.frag"))

    .setDepthStencil({
      .depthTest = false,
      .depthWrite = false,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Front,
    })
    // clang-format off
    .setBlending(0, {
      .enabled = true,
      .srcColor = rhi::BlendFactor::SrcAlpha,
      .dstColor = rhi::BlendFactor::OneMinusSrcAlpha,
      .srcAlpha = rhi::BlendFactor::SrcAlpha,
      .dstAlpha = rhi::BlendFactor::OneMinusSrcAlpha,
    })
    // clang-format on
    .build(getRenderDevice());
}

} // namespace gfx
