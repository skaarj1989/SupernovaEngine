#include "renderer/InfiniteGridPass.hpp"
#include "rhi/CommandBuffer.hpp"

#include "FrameGraphResourceAccess.hpp"
#include "FrameGraphCommon.hpp"

#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/Camera.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

namespace std {

template <> struct hash<gfx::InfiniteGridPass::PassInfo> {
  auto
  operator()(const gfx::InfiniteGridPass::PassInfo &passInfo) const noexcept {
    size_t h{0};
    hashCombine(h, passInfo.depthFormat, passInfo.colorFormat);
    return h;
  }
};

} // namespace std

namespace gfx {

InfiniteGridPass::InfiniteGridPass(rhi::RenderDevice &rd)
    : rhi::RenderPass<InfiniteGridPass>{rd} {}

uint32_t InfiniteGridPass::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void InfiniteGridPass::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource
InfiniteGridPass::addPass(FrameGraph &fg,
                          const FrameGraphBlackboard &blackboard,
                          FrameGraphResource target) {
  static constexpr auto kPassName = "InfiniteGrid";
  ZoneScopedN(kPassName);

  fg.addCallbackPass(
    kPassName,
    [&blackboard, &target](FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<CameraData>());
      builder.read(blackboard.get<GBufferData>().depth, Attachment{});

      target = builder.write(target, Attachment{.index = 0});
    },
    [this](const auto &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      const auto *pipeline = _getPipeline(PassInfo{
        .depthFormat = rhi::getDepthFormat(*framebufferInfo),
        .colorFormat = rhi::getColorFormat(*framebufferInfo, 0),
      });
      if (pipeline) renderFullScreenPostProcess(rc, *pipeline);
    });

  return target;
}

//
// (private):
//

rhi::GraphicsPipeline
InfiniteGridPass::_createPipeline(const PassInfo &passInfo) const {
  ShaderCodeBuilder shaderCodeBuilder;

  return rhi::GraphicsPipeline::Builder{}
    .setDepthFormat(passInfo.depthFormat)
    .setColorFormats({passInfo.colorFormat})
    .setInputAssembly({})
    .setTopology(rhi::PrimitiveTopology::TriangleList)
    .addShader(rhi::ShaderType::Vertex,
               shaderCodeBuilder.buildFromFile("InfiniteGrid.vert"))
    .addShader(rhi::ShaderType::Fragment,
               shaderCodeBuilder.buildFromFile("InfiniteGrid.frag"))

    .setDepthStencil({
      .depthTest = true,
      .depthWrite = false,
      .depthCompareOp = rhi::CompareOp::LessOrEqual,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Front,
    })
    // clang-format off
    .setBlending(0, {
      .enabled = true,
      .srcColor = rhi::BlendFactor::SrcAlpha,
      .dstColor = rhi::BlendFactor::One,
      .colorOp = rhi::BlendOp::Add,

      .srcAlpha = rhi::BlendFactor::Zero,
      .dstAlpha = rhi::BlendFactor::One,
      .alphaOp = rhi::BlendOp::Add,
    })
    // clang-format on
    .build(getRenderDevice());
}

} // namespace gfx
