#include "renderer/SkyboxPass.hpp"
#include "rhi/CommandBuffer.hpp"

#include "FrameGraphResourceAccess.hpp"
#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphTexture.hpp"

#include "FrameGraphData/SkyLight.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/GBuffer.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

namespace std {

template <> struct hash<gfx::SkyboxPass::PassInfo> {
  auto operator()(const gfx::SkyboxPass::PassInfo &passInfo) const noexcept {
    size_t h{0};
    hashCombine(h, passInfo.depthFormat, passInfo.colorFormat,
                passInfo.textureType);
    return h;
  }
};

} // namespace std

namespace gfx {

SkyboxPass::SkyboxPass(rhi::RenderDevice &rd)
    : rhi::RenderPass<SkyboxPass>{rd} {}

uint32_t SkyboxPass::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void SkyboxPass::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource SkyboxPass::addPass(FrameGraph &fg,
                                       const FrameGraphBlackboard &blackboard,
                                       FrameGraphResource target) {
  auto skyLight = blackboard.try_get<SkyLightData>();
  if (!skyLight) return target;

  constexpr auto kPassName = "SkyboxPass";
  ZoneScopedN(kPassName);

  const auto skybox = skyLight->environment;
  assert(skybox);

  fg.addCallbackPass(
    kPassName,
    [&blackboard, &target, skybox](FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<CameraData>(),
           PipelineStage::FragmentShader);
      builder.read(blackboard.get<GBufferData>().depth, Attachment{});
      builder.read(skybox, TextureRead{
                             .binding =
                               {
                                 .location = {.set = 2, .binding = 0},
                                 .pipelineStage = PipelineStage::FragmentShader,
                               },
                             .type = TextureRead::Type::CombinedImageSampler,
                           });

      target = builder.write(target, Attachment{.index = 0});
    },
    [this, skybox](const auto &, FrameGraphPassResources &resources,
                   void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      const auto *pipeline = _getPipeline(PassInfo{
        .depthFormat = rhi::getDepthFormat(*framebufferInfo),
        .colorFormat = rhi::getColorFormat(*framebufferInfo, 0),
        .textureType =
          resources.get<FrameGraphTexture>(skybox).texture->getType(),
      });
      if (pipeline) renderFullScreenPostProcess(rc, *pipeline);
    });

  return target;
}

//
// (private):
//

rhi::GraphicsPipeline
SkyboxPass::_createPipeline(const PassInfo &passInfo) const {
  ShaderCodeBuilder shaderCodeBuilder;
  shaderCodeBuilder.addDefine<int32_t>(
    "CUBEMAP", passInfo.textureType == rhi::TextureType::TextureCube);

  return rhi::GraphicsPipeline::Builder{}
    .setDepthFormat(passInfo.depthFormat)
    .setColorFormats({passInfo.colorFormat})
    .setInputAssembly({})
    .addShader(rhi::ShaderType::Vertex,
               shaderCodeBuilder.buildFromFile("Skybox.vert"))
    .addShader(rhi::ShaderType::Fragment,
               shaderCodeBuilder.buildFromFile("Skybox.frag"))

    .setDepthStencil({
      .depthTest = true,
      .depthWrite = false,
      .depthCompareOp = rhi::CompareOp::LessOrEqual,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Front,
    })
    .setBlending(0, {.enabled = false})
    .build(getRenderDevice());
}

} // namespace gfx
