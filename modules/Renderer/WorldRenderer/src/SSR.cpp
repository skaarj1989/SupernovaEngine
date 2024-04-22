#include "renderer/SSR.hpp"
#include "rhi/CommandBuffer.hpp"

#include "renderer/CommonSamplers.hpp"
#include "renderer/PostProcess.hpp"

#include "FrameGraphResourceAccess.hpp"
#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphTexture.hpp"

#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/SceneColor.hpp"
#include "FrameGraphData/Reflections.hpp"

#include "RenderContext.hpp"

namespace gfx {

SSR::SSR(rhi::RenderDevice &rd, const CommonSamplers &commonSamplers)
    : rhi::RenderPass<SSR>{rd}, m_samplers{commonSamplers} {}

uint32_t SSR::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void SSR::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource SSR::addPass(FrameGraph &fg,
                                FrameGraphBlackboard &blackboard) {
  static constexpr auto kPassName = "SSR";
  ZoneScopedN(kPassName);

  const auto [reflections] = blackboard.add<ReflectionsData>() =
    fg.addCallbackPass<ReflectionsData>(
      kPassName,
      [&fg, &blackboard](FrameGraph::Builder &builder, ReflectionsData &data) {
        PASS_SETUP_ZONE;

        read(builder, blackboard.get<CameraData>(),
             PipelineStage::FragmentShader);
        read(builder, blackboard.get<GBufferData>(),
             GBufferFlags::Depth | GBufferFlags::Normal |
               GBufferFlags::MetallicRoughnessAO | GBufferFlags::Misc);

        const auto sceneColorHDR = blackboard.get<SceneColorData>().HDR;
        builder.read(sceneColorHDR,
                     TextureRead{
                       .binding =
                         {
                           .location = {.set = 1, .binding = 11},
                           .pipelineStage = PipelineStage::FragmentShader,
                         },
                       .type = TextureRead::Type::SampledImage,
                     });

        const auto &inputDesc =
          fg.getDescriptor<FrameGraphTexture>(sceneColorHDR);
        data.reflections =
          builder.create<FrameGraphTexture>("Reflections", inputDesc);
        data.reflections = builder.write(
          data.reflections, Attachment{
                              .index = 0,
                              .clearValue = ClearValue::TransparentBlack,
                            });
      },
      [this](const ReflectionsData &, const FrameGraphPassResources &,
             void *ctx) {
        auto &rc = *static_cast<RenderContext *>(ctx);
        auto &[cb, framebufferInfo, sets] = rc;
        RHI_GPU_ZONE(cb, kPassName);

        const auto *pipeline =
          _getPipeline(rhi::getColorFormat(*framebufferInfo, 0));
        if (pipeline) {
          sets[0][3] = rhi::bindings::SeparateSampler{m_samplers.point};
          renderFullScreenPostProcess(rc, *pipeline);
        }
      });

  return reflections;
}

//
// (private):
//

rhi::GraphicsPipeline
SSR::_createPipeline(const rhi::PixelFormat colorFormat) const {
  return createPostProcessPipelineFromFile(getRenderDevice(), colorFormat,
                                           "SSR.frag");
}

} // namespace gfx
