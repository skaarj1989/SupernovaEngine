#include "renderer/DeferredLightingPass.hpp"
#include "rhi/CommandBuffer.hpp"

#include "renderer/CommonSamplers.hpp"
#include "LightingSettings.hpp"
#include "LightingPassFeatures.hpp"

#include "FrameGraphResourceAccess.hpp"
#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphTexture.hpp"

#include "FrameGraphData/DummyResources.hpp"
#include "FrameGraphData/BRDF.hpp"
#include "FrameGraphData/SkyLight.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/Lights.hpp"
#include "FrameGraphData/LightCulling.hpp"
#include "FrameGraphData/ShadowMap.hpp"
#include "FrameGraphData/GlobalIllumination.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/SSAO.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

namespace gfx {

DeferredLightingPass::DeferredLightingPass(rhi::RenderDevice &rd,
                                           const CommonSamplers &commonSamplers)
    : rhi::RenderPass<DeferredLightingPass>{rd}, m_samplers{commonSamplers} {}

uint32_t DeferredLightingPass::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void DeferredLightingPass::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource DeferredLightingPass::addPass(
  FrameGraph &fg, const FrameGraphBlackboard &blackboard,
  const LightingSettings &lightingSettings, const bool softShadows,
  const bool irradianceOnly) {
  static constexpr auto kPassName = "DeferredLighting Pass";
  ZoneScopedN(kPassName);

  LightingPassFeatures features{
    .softShadows = softShadows,
    .irradianceOnly = irradianceOnly,
  };
  getLightingPassFeatures(features, blackboard);

  struct Data {
    FrameGraphResource sceneColor;
  };
  const auto [sceneColor] = fg.addCallbackPass<Data>(
    kPassName,
    [&fg, &blackboard, &features](FrameGraph::Builder &builder, Data &data) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<CameraData>(),
           PipelineStage::FragmentShader);
      if (features.skyLight) {
        read(builder, blackboard.get<BRDF>());
        read(builder, blackboard.get<SkyLightData>());
      }

      const auto &gBuffer = blackboard.get<GBufferData>();
      read(builder, gBuffer, GBufferFlags::All);
      read(builder, blackboard.get<LightsData>(),
           PipelineStage::FragmentShader);
      if (features.tileSize) {
        read(builder, blackboard.get<LightCullingData>());
      }
      if (features.ssao) {
        read(builder, blackboard.get<SSAOData>());
      }
      const auto &dummyResources = blackboard.get<DummyResourcesData>();
      read(builder, blackboard.get<ShadowMapData>(), dummyResources);
      if (features.globalIllumination) {
        read(builder, blackboard.get<GlobalIlluminationData>());
      }

      const auto inputExtent =
        fg.getDescriptor<FrameGraphTexture>(gBuffer.depth).extent;
      data.sceneColor = builder.create<FrameGraphTexture>(
        "SceneColorHDR", {
                           .extent = inputExtent,
                           .format = rhi::PixelFormat::RGBA16F,
                           .usageFlags = rhi::ImageUsage::RenderTarget |
                                         rhi::ImageUsage::Sampled,
                         });
      data.sceneColor = builder.write(
        data.sceneColor, Attachment{
                           .index = 0,
                           .imageAspect = rhi::ImageAspect::Color,
                           .clearValue = ClearValue::TransparentBlack,
                         });
    },
    [this, lightingSettings,
     features](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      const auto *pipeline =
        _getPipeline(rhi::getColorFormat(*framebufferInfo, 0), features);
      if (pipeline) {
        auto &samplerBindings = sets[0];
        samplerBindings[3] = rhi::bindings::SeparateSampler{m_samplers.point};
        samplerBindings[4] =
          rhi::bindings::SeparateSampler{m_samplers.bilinear};
        samplerBindings[5] = rhi::bindings::SeparateSampler{m_samplers.shadow};
        samplerBindings[6] =
          rhi::bindings::SeparateSampler{m_samplers.omniShadow};

        cb.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);
        cb.pushConstants(rhi::ShaderStages::Fragment, 0, &lightingSettings)
          .beginRendering(*framebufferInfo)
          .drawFullScreenTriangle();
        endRendering(rc);
      }
    });

  return sceneColor;
}

//
// (private):
//

rhi::GraphicsPipeline DeferredLightingPass::_createPipeline(
  const rhi::PixelFormat colorFormat,
  const LightingPassFeatures &features) const {
  ShaderCodeBuilder shaderCodeBuilder;
  const auto vertCode =
    shaderCodeBuilder.buildFromFile("FullScreenTriangle.vert");

  addLighting(shaderCodeBuilder, features);
  const auto fragCode =
    shaderCodeBuilder.buildFromFile("DeferredLighting.frag");

  return rhi::GraphicsPipeline::Builder{}
    .setColorFormats({colorFormat})
    .setInputAssembly({})
    .addShader(rhi::ShaderType::Vertex, vertCode)
    .addShader(rhi::ShaderType::Fragment, fragCode)

    .setDepthStencil({
      .depthTest = false,
      .depthWrite = false,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Front,
    })
    .setBlending(0, {.enabled = false})
    .build(getRenderDevice());
}

} // namespace gfx
