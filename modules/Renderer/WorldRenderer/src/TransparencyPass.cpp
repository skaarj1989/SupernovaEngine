#include "renderer/TransparencyPass.hpp"

#include "FrameGraphForwardPass.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "renderer/CommonSamplers.hpp"

#include "MaterialShader.hpp"
#include "BatchBuilder.hpp"
#include "UploadInstances.hpp"

#include "RenderContext.hpp"

namespace gfx {

namespace {

[[nodiscard]] auto canDraw(const Renderable *renderable) {
  assert(renderable);

  const auto &surface =
    *renderable->subMeshInstance.material->getBlueprint().surface;
  switch (surface.blendMode) {
    using enum BlendMode;

  case Transparent:
  case Add:
  case Modulate:
    return true;
  }
  return false;
};

[[nodiscard]] auto batchCompatible(const Batch &b, const Renderable &r) {
  return sameGeometry(b, r) && sameMaterial(b, r) && sameTextures(b, r);
}

[[nodiscard]] rhi::BlendState getBlendState(BlendMode blendMode) {
  switch (blendMode) {
  case BlendMode::Transparent:
    return {
      .enabled = true,
      .srcColor = rhi::BlendFactor::SrcAlpha,
      .dstColor = rhi::BlendFactor::OneMinusSrcAlpha,
      .colorOp = rhi::BlendOp::Add,
      .srcAlpha = rhi::BlendFactor::One,
      .dstAlpha = rhi::BlendFactor::Zero,
      .alphaOp = rhi::BlendOp::Add,
    };
  case BlendMode::Add:
    return {
      .enabled = true,
      .srcColor = rhi::BlendFactor::One,
      .dstColor = rhi::BlendFactor::One,
      .colorOp = rhi::BlendOp::Add,
      .srcAlpha = rhi::BlendFactor::One,
      .dstAlpha = rhi::BlendFactor::One,
      .alphaOp = rhi::BlendOp::Add,
    };

  case BlendMode::Modulate:
    return {
      .enabled = true,
      .srcColor = rhi::BlendFactor::One,
      .dstColor = rhi::BlendFactor::Zero,
      .colorOp = rhi::BlendOp::Add,
      .srcAlpha = rhi::BlendFactor::One,
      .dstAlpha = rhi::BlendFactor::Zero,
      .alphaOp = rhi::BlendOp::Add,
    };
  }

  return {.enabled = false};
}

} // namespace

//
// TransparencyPass class:
//

TransparencyPass::TransparencyPass(rhi::RenderDevice &rd,
                                   const CommonSamplers &commonSamplers)
    : rhi::RenderPass<TransparencyPass>{rd}, m_samplers{commonSamplers} {}

uint32_t TransparencyPass::count(PipelineGroups flags) const {
  return bool(flags & PipelineGroups::SurfaceMaterial) ? BasePass::count() : 0;
}
void TransparencyPass::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::SurfaceMaterial)) BasePass::clear();
}

std::optional<FrameGraphResource> TransparencyPass::addGeometryPass(
  FrameGraph &fg, const FrameGraphBlackboard &blackboard,
  FrameGraphResource sceneColor, const ViewInfo &viewData,
  const PropertyGroupOffsets &propertyGroupOffsets,
  const LightingSettings &lightingSettings, bool softShadows) {
  ZoneScoped;

  constexpr auto kPassName = "ForwardTransparency";

  std::vector<const Renderable *> transparentRenderables;
  transparentRenderables.reserve(viewData.visibleRenderables.size());
  std::ranges::copy_if(viewData.visibleRenderables,
                       std::back_inserter(transparentRenderables), canDraw);
  if (transparentRenderables.empty()) return std::nullopt;

  sortByMaterial(transparentRenderables);

  std::vector<GPUInstance> gpuInstances;
  auto batches = buildBatches(gpuInstances, transparentRenderables,
                              propertyGroupOffsets, batchCompatible);
  if (batches.empty()) return std::nullopt;

  const auto instances = *uploadInstances(fg, std::move(gpuInstances));

  LightingPassFeatures features{.softShadows = softShadows};
  getLightingPassFeatures(features, blackboard);

  struct Data {
    FrameGraphResource output;
  };
  const auto [output] = fg.addCallbackPass<Data>(
    kPassName,
    [&fg, &blackboard, sceneColor, instances](FrameGraph::Builder &builder,
                                              Data &data) {
      read(builder, blackboard, instances);

      const auto &sceneColorDesc =
        fg.getDescriptor<FrameGraphTexture>(sceneColor);
      data.output = builder.create<FrameGraphTexture>(
        "SceneColor w/ Transparency", sceneColorDesc);
      data.output =
        builder.write(data.output, Attachment{
                                     .index = 0,
                                     .clearValue = ClearValue::TransparentBlack,
                                   });
    },
    [this, lightingSettings, features, batches = std::move(batches)](
      const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, kPassName)

      auto &[cb, framebufferInfo, sets] = rc;
      auto &samplerBindings = sets[0];
      samplerBindings[4] = rhi::bindings::SeparateSampler{m_samplers.shadow};
      samplerBindings[5] =
        rhi::bindings::SeparateSampler{m_samplers.omniShadow};

      overrideSampler(sets[1][5], m_samplers.bilinear);
      overrideSampler(sets[1][11], m_samplers.bilinear);

      BaseGeometryPassInfo passInfo{
        .depthFormat = rhi::getDepthFormat(*framebufferInfo),
        .colorFormats = rhi::getColorFormats(*framebufferInfo),
      };

      cb.beginRendering(*framebufferInfo);
      for (const auto &batch : batches) {
        const auto &pipeline =
          _getPipeline(ForwardPassInfo{adjust(passInfo, batch), features});
        if (pipeline) {
          bindBatch(rc, batch);
          cb.bindPipeline(*pipeline);
          bindDescriptorSets(rc, *pipeline);
          cb.pushConstants(rhi::ShaderStages::Fragment, 16, &lightingSettings);
          drawBatch(rc, batch);
        }
      }
      endRendering(rc);
    });

  return output;
}

CodePair TransparencyPass::buildShaderCode(
  const rhi::RenderDevice &rd, const VertexFormat *vertexFormat,
  const Material &material, const LightingPassFeatures &features) {
  const auto offsetAlignment =
    rd.getDeviceLimits().minStorageBufferOffsetAlignment;

  CodePair code;

  const auto commonDefines = buildDefines(*vertexFormat);

  ShaderCodeBuilder shaderCodeBuilder;

  // -- VertexShader:

  shaderCodeBuilder.setDefines(commonDefines);
  addMaterial(shaderCodeBuilder, material, rhi::ShaderType::Vertex,
              offsetAlignment);
  code.vert = shaderCodeBuilder.buildFromFile("Mesh.vert");

  // -- FragmentShader:

  shaderCodeBuilder.setDefines(commonDefines)
    .addDefine("HAS_SCENE_DEPTH", 1)
    .addDefine("HAS_SCENE_COLOR", 1);
  addMaterial(shaderCodeBuilder, material, rhi::ShaderType::Fragment,
              offsetAlignment);
  addLighting(shaderCodeBuilder, features);
  code.frag = shaderCodeBuilder.buildFromFile("ForwardPass.frag");

  return code;
}

//
// (private):
//

rhi::GraphicsPipeline
TransparencyPass::_createPipeline(const ForwardPassInfo &passInfo) const {
  assert(passInfo.vertexFormat && passInfo.material);

  auto &rd = getRenderDevice();

  const auto &material = *passInfo.material;
  const auto [vertCode, fragCode] =
    buildShaderCode(rd, passInfo.vertexFormat, material, passInfo.features);

  const auto &surface = getSurface(material);

  return rhi::GraphicsPipeline::Builder{}
    .setDepthFormat(passInfo.depthFormat)
    .setColorFormats(passInfo.colorFormats)
    .setInputAssembly(passInfo.vertexFormat->getAttributes())
    .setTopology(passInfo.topology)
    .addShader(rhi::ShaderType::Vertex, vertCode)
    .addShader(rhi::ShaderType::Fragment, fragCode)

    .setDepthStencil({
      .depthTest = true,
      .depthWrite = false,
      .depthCompareOp = rhi::CompareOp::LessOrEqual,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = surface.cullMode,
    })
    .setBlending(0, getBlendState(surface.blendMode))
    .build(rd);
}

} // namespace gfx
