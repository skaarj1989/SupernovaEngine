#include "renderer/PostProcessor.hpp"
#include "rhi/RenderDevice.hpp"

#include "renderer/CommonSamplers.hpp"
#include "renderer/PostProcess.hpp"

#include "renderer/MaterialInstance.hpp"

#include "FrameGraphResourceAccess.hpp"
#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "UploadMaterialProperties.hpp"

#include "FrameGraphData/Frame.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/MaterialProperties.hpp"
#include "FrameGraphData/GBuffer.hpp"

#include "MaterialShader.hpp"
#include "BuildPropertyBuffer.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

namespace std {

template <> struct hash<gfx::PostProcessor::PassInfo> {
  auto operator()(const gfx::PostProcessor::PassInfo &passInfo) const noexcept {
    assert(passInfo.material);

    size_t h{0};
    hashCombine(h, passInfo.colorFormat, passInfo.material->getHash());
    return h;
  }
};

} // namespace std

namespace gfx {

namespace {

[[nodiscard]] auto
getMaterialProperties(FrameGraph &fg, const MaterialInstance &materialInstance,
                      const rhi::RenderDevice &rd) {
  ZoneScopedN("GetMaterialProperties");
  std::optional<FrameGraphResource> materialProperties;
  if (auto material = materialInstance.getPrototype();
      hasProperties(*material)) {
    const auto minOffsetAlignment =
      rd.getDeviceLimits().minStorageBufferOffsetAlignment;
    auto buffer =
      buildPropertyBuffer(material->getPropertyLayout(),
                          materialInstance.getProperties(), minOffsetAlignment);
    materialProperties = uploadMaterialProperties(fg, std::move(buffer));
  }
  return materialProperties;
}

} // namespace

//
// PostProcessor class:
//

PostProcessor::PostProcessor(rhi::RenderDevice &rd,
                             const CommonSamplers &commonSamplers)
    : rhi::RenderPass<PostProcessor>{rd}, m_samplers{commonSamplers} {}

uint32_t PostProcessor::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::PostProcessMaterial) ? BasePass::count()
                                                           : 0;
}
void PostProcessor::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::PostProcessMaterial)) BasePass::clear();
}

FrameGraphResource
PostProcessor::addPass(FrameGraph &fg, const FrameGraphBlackboard &blackboard,
                       const FrameGraphResource sceneColor,
                       const MaterialInstance &material) {
  if (!material) return sceneColor;

  std::string passName{material->getName()};
  if (passName.empty()) passName = "(Unknown) UserPostProcess";

  ZoneTransientN(__tracy_zone, passName.c_str(), true);

  const auto materialProperties =
    getMaterialProperties(fg, material, getRenderDevice());

  struct Data {
    FrameGraphResource output;
  };
  const auto [output] = fg.addCallbackPass<Data>(
    passName,
    [&fg, &blackboard, sceneColor,
     materialProperties](FrameGraph::Builder &builder, Data &data) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<FrameData>(), PipelineStage::FragmentShader);
      read(builder, blackboard.get<CameraData>(),
           PipelineStage::FragmentShader);

      builder.read(blackboard.get<GBufferData>().depth,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 2, .binding = 0},
                         .pipelineStage = PipelineStage::FragmentShader,
                       },
                     .type = TextureRead::Type::CombinedImageSampler,
                     .imageAspect = rhi::ImageAspect::Depth,
                   });
      builder.read(sceneColor,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 2, .binding = 1},
                         .pipelineStage = PipelineStage::FragmentShader,
                       },
                     .type = TextureRead::Type::CombinedImageSampler,
                     .imageAspect = rhi::ImageAspect::Color,
                   });

      if (materialProperties)
        read(builder, MaterialPropertiesData{*materialProperties});

      const auto &sceneColorDesc =
        fg.getDescriptor<FrameGraphTexture>(sceneColor);
      data.output =
        builder.create<FrameGraphTexture>("SceneColor", sceneColorDesc);
      data.output =
        builder.write(data.output, Attachment{
                                     .index = 0,
                                     .imageAspect = rhi::ImageAspect::Color,
                                     .clearValue = ClearValue::TransparentBlack,
                                   });
    },
    [this, materialPtr = &material,
     passName](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, passName.c_str());

      const auto *pipeline = _getPipeline(PassInfo{
        .colorFormat = rhi::getColorFormat(*framebufferInfo, 0),
        .material = materialPtr->getPrototype().get(),
      });
      if (pipeline) {
        auto &samplerBindings = sets[2];
        overrideSampler(samplerBindings[0], m_samplers.bilinear);
        overrideSampler(samplerBindings[1], m_samplers.bilinear);

        bindMaterialTextures(rc, materialPtr->getTextures());
        renderFullScreenPostProcess(rc, *pipeline);
      }
    });

  return output;
}

//
// (private):
//

rhi::GraphicsPipeline
PostProcessor::_createPipeline(const PassInfo &passInfo) const {
  assert(passInfo.material);

  auto &rd = getRenderDevice();
  return createPostProcessPipeline(rd, passInfo.colorFormat,
                                   buildFragmentCode(rd, *passInfo.material));
}

std::string PostProcessor::buildFragmentCode(const rhi::RenderDevice &rd,
                                             const Material &material) {
  const auto offsetAlignment =
    rd.getDeviceLimits().minStorageBufferOffsetAlignment;

  ShaderCodeBuilder shaderCodeBuilder;
  shaderCodeBuilder.addDefine("HAS_SCENE_DEPTH", 1)
    .addDefine("HAS_SCENE_COLOR", 1);
  addMaterial(shaderCodeBuilder, material, rhi::ShaderType::Fragment,
              offsetAlignment);

  return shaderCodeBuilder.buildFromFile("PostProcess.frag");
}

} // namespace gfx
