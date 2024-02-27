#include "renderer/FinalPass.hpp"

#include "renderer/FrameGraphTexture.hpp"
#include "FrameGraphCommon.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/LightCulling.hpp"
#include "FrameGraphData/WeightedBlended.hpp"
#include "FrameGraphData/SceneColor.hpp"
#include "FrameGraphData/SSAO.hpp"
#include "FrameGraphData/Reflections.hpp"
#include "FrameGraphData/BrightColor.hpp"

#include "renderer/CommonSamplers.hpp"

#include "renderer/PostProcess.hpp"
#include "RenderContext.hpp"

namespace gfx {

namespace {

enum class Mode {
  Default = 0,
  LinearDepth,
  RedChannel,
  GreenChannel,
  BlueChannel,
  AlphaChannel,
  ViewSpaceNormals,
  WorldSpaceNormals,
};

[[nodiscard]] auto pickOutput(const FrameGraphBlackboard &blackboard,
                              OutputMode outputMode) {
  auto mode{Mode::Default};

  std::optional<FrameGraphResource> input;
  switch (outputMode) {
    using enum OutputMode;

  case Depth:
    input = blackboard.get<GBufferData>().depth;
    mode = Mode::LinearDepth;
    break;
  case Normal:
    input = blackboard.get<GBufferData>().normal;
    mode = Mode::WorldSpaceNormals;
    break;
  case Emissive:
    input = blackboard.get<GBufferData>().emissive;
    break;
  case BaseColor:
    input = blackboard.get<GBufferData>().albedo;
    break;
  case Metallic:
    input = blackboard.get<GBufferData>().metallicRoughnessAO;
    mode = Mode::RedChannel;
    break;
  case Roughness:
    input = blackboard.get<GBufferData>().metallicRoughnessAO;
    mode = Mode::GreenChannel;
    break;
  case AmbientOcclusion:
    input = blackboard.get<GBufferData>().metallicRoughnessAO;
    mode = Mode::BlueChannel;
    break;

  case SSAO:
    if (auto *d = blackboard.try_get<SSAOData>()) {
      input = d->ssao;
      mode = Mode::RedChannel;
    }
    break;
  case BrightColor:
    if (auto *d = blackboard.try_get<BrightColorData>()) {
      input = d->brightColor;
    }
    break;
  case Reflections:
    if (auto *d = blackboard.try_get<ReflectionsData>()) {
      input = d->reflections;
    }
    break;

  case Accum:
    if (auto *d = blackboard.try_get<WeightedBlendedData>(); d) {
      input = d->accum;
    }
    break;
  case Reveal:
    if (auto *d = blackboard.try_get<WeightedBlendedData>(); d) {
      input = d->reveal;
      mode = Mode::RedChannel;
    }
    break;

  case LightHeatmap:
    if (auto *d = blackboard.try_get<LightCullingData>(); d && d->debugMap) {
      input = *d->debugMap;
    }
    break;

  case HDR:
    input = blackboard.get<SceneColorData>().HDR;
    break;
  case FinalImage:
    input = blackboard.get<SceneColorData>().LDR;
    break;

  default:
    assert(false);
    break;
  }
  return std::tuple{input, mode};
}

} // namespace

FinalPass::FinalPass(rhi::RenderDevice &rd,
                     const CommonSamplers &commonSamplers)
    : rhi::RenderPass<FinalPass>{rd}, m_samplers{commonSamplers} {}

uint32_t FinalPass::count(PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void FinalPass::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource FinalPass::compose(FrameGraph &fg,
                                      const FrameGraphBlackboard &blackboard,
                                      OutputMode outputMode,
                                      FrameGraphResource target) {
  constexpr auto kPassName = "FinalComposition";
  ZoneScopedN(kPassName);

  auto [source, mode] = pickOutput(blackboard, outputMode);

  fg.addCallbackPass(
    kPassName,
    [&blackboard, &target, source](FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<CameraData>(),
           PipelineStage::FragmentShader);
      if (source) {
        builder.read(*source,
                     TextureRead{
                       .binding =
                         {
                           .location = {.set = 2, .binding = 1},
                           .pipelineStage = PipelineStage::FragmentShader,
                         },
                       .type = TextureRead::Type::CombinedImageSampler,
                     });
      }

      target = builder.write(target, Attachment{
                                       .index = 0,
                                       .clearValue = ClearValue::OpaqueBlack,
                                     });
    },
    [this, target, source,
     mode](const auto &, FrameGraphPassResources &resources, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      const auto *pipeline =
        _getPipeline(rhi::getColorFormat(*framebufferInfo, 0),
                     source ? Mode::Final : Mode::Pattern);
      if (pipeline) {
        cb.bindPipeline(*pipeline);
        if (source) {
          overrideSampler(sets[2][1], m_samplers.bilinear);

          bindDescriptorSets(rc, *pipeline);
          cb.pushConstants(rhi::ShaderStages::Fragment, 0, &mode);
        }
        cb.beginRendering(*framebufferInfo).drawFullScreenTriangle();
        endRendering(rc);
      }
      rhi::prepareForReading(cb,
                             *resources.get<FrameGraphTexture>(target).texture);
    });

  return target;
}

rhi::GraphicsPipeline FinalPass::_createPipeline(rhi::PixelFormat colorFormat,
                                                 Mode mode) const {
  return createPostProcessPipelineFromFile(
    getRenderDevice(), colorFormat,
    mode == Mode::Final ? "FinalPass.frag" : "Pattern.frag");
}

} // namespace gfx
