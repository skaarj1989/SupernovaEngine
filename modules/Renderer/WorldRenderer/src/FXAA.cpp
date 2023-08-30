#include "renderer/FXAA.hpp"

#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "FrameGraphData/Camera.hpp"

#include "renderer/CommonSamplers.hpp"

#include "renderer/PostProcess.hpp"
#include "RenderContext.hpp"

namespace gfx {

FXAA::FXAA(rhi::RenderDevice &rd, const CommonSamplers &commonSamplers)
    : rhi::RenderPass<FXAA>{rd}, m_samplers{commonSamplers} {}

uint32_t FXAA::count(PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void FXAA::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource FXAA::addPass(FrameGraph &fg,
                                 const FrameGraphBlackboard &blackboard,
                                 FrameGraphResource input) {
  ZoneScoped;

  constexpr auto kPassName = "FXAA";

  struct Data {
    FrameGraphResource output;
  };
  const auto [output] = fg.addCallbackPass<Data>(
    kPassName,
    [&fg, &blackboard, input](FrameGraph::Builder &builder, Data &data) {
      read(builder, blackboard.get<CameraData>(),
           PipelineStage::FragmentShader);
      builder.read(input, TextureRead{
                            .binding =
                              {
                                .location = {.set = 2, .binding = 0},
                                .pipelineStage = PipelineStage::FragmentShader,
                              },
                            .type = TextureRead::Type::CombinedImageSampler,
                          });

      const auto &inputDesc = fg.getDescriptor<FrameGraphTexture>(input);
      data.output = builder.create<FrameGraphTexture>("AA", inputDesc);
      data.output = builder.write(data.output, Attachment{.index = 0});
    },
    [this](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, kPassName)

      auto &[cb, framebufferInfo, sets] = rc;
      const auto *pipeline =
        _getPipeline(rhi::getColorFormat(*framebufferInfo, 0));
      if (pipeline) {
        overrideSampler(sets[2][0], m_samplers.bilinear);
        renderFullScreenPostProcess(rc, *pipeline);
      }
    });

  return output;
}

//
// (private):
//

rhi::GraphicsPipeline
FXAA::_createPipeline(rhi::PixelFormat colorFormat) const {
  return createPostProcessPipelineFromFile(getRenderDevice(), colorFormat,
                                           "FXAA.frag");
}

} // namespace gfx
