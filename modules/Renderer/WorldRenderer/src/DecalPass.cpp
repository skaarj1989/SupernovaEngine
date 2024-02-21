#include "renderer/DecalPass.hpp"

#include "FrameGraphCommon.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "FrameGraphData/Frame.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/Transforms.hpp"
#include "FrameGraphData/MaterialProperties.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/DummyResources.hpp"

#include "renderer/CommonSamplers.hpp"

#include "MaterialShader.hpp"
#include "BatchBuilder.hpp"
#include "UploadInstances.hpp"

#include "RenderContext.hpp"

namespace gfx {

// KNOWN ISSUES:
// - A decal disappears when intersecting a camera (near plane).

namespace {

void markDecal(ShaderCodeBuilder &builder) { builder.addDefine("IS_DECAL", 1); }

void addDecalBlendMode(ShaderCodeBuilder &builder,
                       const DecalBlendMode &decalBlendMode) {
  markDecal(builder);
  builder.addDefine<int32_t>("WRITE_NORMAL", bool(decalBlendMode.normal))
    .addDefine<int32_t>("WRITE_EMISSIVE", bool(decalBlendMode.emissive))
    .addDefine<int32_t>("WRITE_ALBEDO", bool(decalBlendMode.albedo))
    .addDefine<int32_t>("WRITE_METALLIC_ROUGHNESS_AO",
                        bool(decalBlendMode.metallicRoughnessAO));
}

[[nodiscard]] auto batchCompatible(const Batch &b, const Renderable &r) {
  return sameGeometry(b, r) && sameMaterial(b, r) && sameTextures(b, r);
}

} // namespace

DecalPass::DecalPass(rhi::RenderDevice &rd,
                     const CommonSamplers &commonSamplers)
    : rhi::RenderPass<DecalPass>{rd}, m_samplers{commonSamplers} {}

uint32_t DecalPass::count(PipelineGroups flags) const {
  return bool(flags & PipelineGroups::SurfaceMaterial) ? BasePass::count() : 0;
}
void DecalPass::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::SurfaceMaterial)) BasePass::clear();
}

void DecalPass::addGeometryPass(
  FrameGraph &fg, FrameGraphBlackboard &blackboard, const ViewInfo &viewData,
  const PropertyGroupOffsets &propertyGroupOffsets) {
  assert(!viewData.visibleRenderables.empty());

  ZoneScoped;

  constexpr auto kPassName = "DecalPass";

  std::vector<GPUInstance> gpuInstances;
  auto batches = buildBatches(gpuInstances, viewData.visibleRenderables,
                              propertyGroupOffsets, batchCompatible);
  if (batches.empty()) return;

  const auto instances = *uploadInstances(fg, std::move(gpuInstances));

  fg.addCallbackPass(
    kPassName,
    [&blackboard, instances](FrameGraph::Builder &builder, auto &) {
      read(builder, blackboard.get<FrameData>());
      read(builder, blackboard.get<CameraData>());

      const auto &dummyResources = blackboard.get<DummyResourcesData>();
      readInstances(builder, instances, dummyResources);
      read(builder, blackboard.try_get<TransformData>(), dummyResources);
      if (const auto *d = blackboard.try_get<MaterialPropertiesData>(); d) {
        read(builder, *d);
      }

      auto &gBuffer = blackboard.get<GBufferData>();
      readSceneDepth(builder, gBuffer.depth,
                     ReadFlags::Attachment | ReadFlags::Sampling);

      gBuffer.normal = builder.write(gBuffer.normal, Attachment{.index = 0});
      gBuffer.emissive =
        builder.write(gBuffer.emissive, Attachment{.index = 1});
      gBuffer.albedo = builder.write(gBuffer.albedo, Attachment{.index = 2});
      gBuffer.metallicRoughnessAO =
        builder.write(gBuffer.metallicRoughnessAO, Attachment{.index = 3});
    },
    [this, batches = std::move(batches)](
      const auto &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, kPassName)

      auto &[cb, framebufferInfo, sets] = rc;
      overrideSampler(sets[1][5], m_samplers.bilinear);

      cb.beginRendering(*framebufferInfo);
      BaseGeometryPassInfo passInfo{
        .depthFormat = rhi::getDepthFormat(*framebufferInfo),
        .colorFormats = rhi::getColorFormats(*framebufferInfo),
      };
      for (const auto &batch : batches) {
        if (const auto *pipeline = _getPipeline(adjust(passInfo, batch));
            pipeline) {
          render(rc, *pipeline, batch);
        }
      }
      endRendering(rc);
    });
}

CodePair DecalPass::buildShaderCode(const rhi::RenderDevice &rd,
                                    const VertexFormat *vertexFormat,
                                    const Material &material) {
  assert(isSurface(material));

  const auto offsetAlignment =
    rd.getDeviceLimits().minStorageBufferOffsetAlignment;

  CodePair code;

  const auto commonDefines =
    vertexFormat ? buildDefines(*vertexFormat) : Defines{};

  ShaderCodeBuilder shaderCodeBuilder;

  // -- VertexShader:

  shaderCodeBuilder.setDefines(commonDefines);
  addMaterial(shaderCodeBuilder, material, rhi::ShaderType::Vertex,
              offsetAlignment);
  markDecal(shaderCodeBuilder);
  code.vert = shaderCodeBuilder.buildFromFile("Decal.vert");

  // -- FragmentShader:

  shaderCodeBuilder.setDefines(commonDefines);
  addMaterial(shaderCodeBuilder, material, rhi::ShaderType::Fragment,
              offsetAlignment);
  addDecalBlendMode(shaderCodeBuilder, getSurface(material).decalBlendMode);
  code.frag = shaderCodeBuilder.buildFromFile("Decal.frag");

  return code;
}

//
// (private):
//

rhi::GraphicsPipeline
DecalPass::_createPipeline(const BaseGeometryPassInfo &passInfo) const {
  assert(passInfo.vertexFormat && passInfo.material);

  const auto &material = *passInfo.material;
  const auto [vertCode, fragCode] =
    buildShaderCode(getRenderDevice(), passInfo.vertexFormat, material);

  constexpr auto pickBlendState = [](BlendOp b) {
    return b == BlendOp::Replace
             ? rhi::BlendState{.enabled = false}
             : rhi::BlendState{
                 .enabled = true,
                 .srcColor = rhi::BlendFactor::One,
                 .dstColor = rhi::BlendFactor::One,
                 .colorOp = rhi::BlendOp::Add,
                 .srcAlpha = rhi::BlendFactor::OneMinusSrcColor,
                 .dstAlpha = rhi::BlendFactor::One,
                 .alphaOp = rhi::BlendOp::Add,
               };
  };

  const auto &decalBlendMode = getSurface(material).decalBlendMode;

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
      .cullMode = rhi::CullMode::Front,
    })
    .setBlending(0, pickBlendState(decalBlendMode.normal))
    .setBlending(1, pickBlendState(decalBlendMode.emissive))
    .setBlending(2, pickBlendState(decalBlendMode.albedo))
    .setBlending(3, pickBlendState(decalBlendMode.metallicRoughnessAO))
    .build(getRenderDevice());
}

} // namespace gfx
