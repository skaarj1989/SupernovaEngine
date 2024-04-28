#include "renderer/TonemapPass.hpp"
#include "rhi/CommandBuffer.hpp"

#include "renderer/CommonSamplers.hpp"
#include "renderer/PostProcess.hpp"

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"
#include "FrameGraphResourceAccess.hpp"
#include "renderer/FrameGraphTexture.hpp"

#include "FrameGraphData/SceneColor.hpp"
#include "FrameGraphData/AverageLuminance.hpp"
#include "FrameGraphData/BrightColor.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

namespace gfx {

TonemapPass::TonemapPass(rhi::RenderDevice &rd,
                         const CommonSamplers &commonSamplers)
    : rhi::RenderPass<TonemapPass>{rd}, m_samplers{commonSamplers} {}

uint32_t TonemapPass::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void TonemapPass::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource TonemapPass::addPass(FrameGraph &fg,
                                        FrameGraphBlackboard &blackboard,
                                        const Tonemap tonemap,
                                        const float exposure,
                                        const float bloomStrength) {
  static constexpr auto kPassName = "Tonemapping";
  ZoneScopedN(kPassName);

  std::optional<FrameGraphResource> averageLuminance;
  if (auto d = blackboard.try_get<AverageLuminanceData>(); d) {
    averageLuminance = d->averageLuminance;
  }
  std::optional<FrameGraphResource> bloom;
  if (auto d = blackboard.try_get<BrightColorData>(); d) bloom = d->brightColor;

  struct Data {
    FrameGraphResource output;
  };
  const auto [output] = fg.addCallbackPass<Data>(
    kPassName,
    [&fg, &blackboard, averageLuminance, bloom](FrameGraph::Builder &builder,
                                                Data &data) {
      PASS_SETUP_ZONE;

      const auto sceneColorHDR = blackboard.get<SceneColorData>().HDR;
      builder.read(sceneColorHDR,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 0, .binding = 1},
                         .pipelineStage = PipelineStage::FragmentShader,
                       },
                     .type = TextureRead::Type::SampledImage,
                     .imageAspect = rhi::ImageAspect::Color,
                   });
      if (averageLuminance) {
        builder.read(*averageLuminance,
                     TextureRead{
                       .binding =
                         {
                           .location = {.set = 0, .binding = 2},
                           .pipelineStage = PipelineStage::FragmentShader,
                         },
                       .type = TextureRead::Type::StorageImage,
                       .imageAspect = rhi::ImageAspect::Color,
                     });
      }
      if (bloom) {
        builder.read(*bloom,
                     TextureRead{
                       .binding =
                         {
                           .location = {.set = 0, .binding = 3},
                           .pipelineStage = PipelineStage::FragmentShader,
                         },
                       .type = TextureRead::Type::SampledImage,
                       .imageAspect = rhi::ImageAspect::Color,
                     });
      }

      const auto inputExtent =
        fg.getDescriptor<FrameGraphTexture>(sceneColorHDR).extent;
      data.output = builder.create<FrameGraphTexture>(
        "SceneColor", {
                        .extent = inputExtent,
                        .format = rhi::PixelFormat::RGBA8_UNorm,
                        .usageFlags = rhi::ImageUsage::RenderTarget |
                                      rhi::ImageUsage::Sampled,
                      });
      data.output = builder.write(
        data.output,
        Attachment{.index = 0, .imageAspect = rhi::ImageAspect::Color});
    },
    [this, averageLuminance, bloom, tonemap, exposure,
     bloomStrength](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      const auto *pipeline =
        _getPipeline(rhi::getColorFormat(*framebufferInfo, 0),
                     averageLuminance.has_value(), bloom.has_value());
      if (pipeline) {
        sets[0][0] = rhi::bindings::SeparateSampler{m_samplers.bilinear};

        cb.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);

        struct Uniforms {
          uint32_t tonemap;
          float exposure;
          float bloomStrength;
        };
        const Uniforms uniforms{
          .tonemap = uint32_t(tonemap),
          .exposure = exposure,
          .bloomStrength = bloomStrength,
        };
        cb.pushConstants(rhi::ShaderStages::Fragment, 0, &uniforms)
          .beginRendering(*framebufferInfo)
          .drawFullScreenTriangle();
        endRendering(rc);
      }
    });

  return output;
}

//
// (private):
//

rhi::GraphicsPipeline
TonemapPass::_createPipeline(const rhi::PixelFormat colorFormat,
                             const bool autoExposure, const bool bloom) const {
  ShaderCodeBuilder shaderCodeBuilder;
  shaderCodeBuilder.addDefine<int32_t>("HAS_EYE_ADAPTATION", autoExposure)
    .addDefine<int32_t>("HAS_BLOOM", bloom);

  return createPostProcessPipeline(
    getRenderDevice(), colorFormat,
    shaderCodeBuilder.buildFromFile("TonemapPass.frag"));
}

} // namespace gfx
