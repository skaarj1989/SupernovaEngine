#include "renderer/GBufferPass.hpp"

#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "FrameGraphData/DummyResources.hpp"
#include "FrameGraphData/Frame.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/Transforms.hpp"
#include "FrameGraphData/Skins.hpp"
#include "FrameGraphData/MaterialProperties.hpp"
#include "FrameGraphData/GBuffer.hpp"

#include "MaterialShader.hpp"
#include "BatchBuilder.hpp"
#include "UploadInstances.hpp"

#include "RenderContext.hpp"

namespace gfx {

namespace {

[[nodiscard]] auto canDraw(const Renderable *renderable) {
  const auto &blueprint = renderable->subMeshInstance.material->getBlueprint();
  const auto &surface = *blueprint.surface;

  using enum LightingMode;
  using enum BlendMode;

  return surface.lightingMode != Transmission && surface.blendMode == Opaque ||
         surface.blendMode == Masked;
};

[[nodiscard]] auto batchCompatible(const Batch &b, const Renderable &r) {
  return sameGeometry(b, r) && sameMaterial(b, r) && sameTextures(b, r);
}

} // namespace

GBufferPass::GBufferPass(rhi::RenderDevice &rd)
    : rhi::RenderPass<GBufferPass>{rd} {}

uint32_t GBufferPass::count(PipelineGroups flags) const {
  return bool(flags & PipelineGroups::SurfaceMaterial) ? BasePass::count() : 0;
}
void GBufferPass::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::SurfaceMaterial)) BasePass::clear();
}

void GBufferPass::addGeometryPass(
  FrameGraph &fg, FrameGraphBlackboard &blackboard, rhi::Extent2D resolution,
  const ViewInfo &viewData, const PropertyGroupOffsets &propertyGroupOffsets) {
  ZoneScoped;

  constexpr auto kPassName = "GBufferPass";

  std::vector<const Renderable *> opaqueRenderables;
  opaqueRenderables.reserve(viewData.visibleRenderables.size());
  std::ranges::copy_if(viewData.visibleRenderables,
                       std::back_inserter(opaqueRenderables), canDraw);
  sortByMaterial(opaqueRenderables);

  std::vector<GPUInstance> gpuInstances;
  auto batches = buildBatches(gpuInstances, opaqueRenderables,
                              propertyGroupOffsets, batchCompatible);
  // (Do not early-exit) Without renderables the pass clears GBuffer
  // attachments (the succeeding passes might need the DepthBuffer).

  const auto instances = uploadInstances(fg, std::move(gpuInstances));

  blackboard.add<GBufferData>() = fg.addCallbackPass<GBufferData>(
    kPassName,
    [&fg, &blackboard, resolution, instances](FrameGraph::Builder &builder,
                                              GBufferData &data) {
      read(builder, blackboard.get<FrameData>());
      read(builder, blackboard.get<CameraData>());

      const auto &dummyResources = blackboard.get<DummyResourcesData>();
      readInstances(builder, instances, dummyResources);
      read(builder, blackboard.try_get<TransformData>(), dummyResources);
      read(builder, blackboard.try_get<SkinData>(), dummyResources);

      if (auto d = blackboard.try_get<MaterialPropertiesData>(); d) {
        read(builder, *d);
      }

      data.depth = builder.create<FrameGraphTexture>(
        "SceneDepth", {
                        .extent = resolution,
                        .format = rhi::PixelFormat::Depth32F,
                        .usageFlags = rhi::ImageUsage::RenderTarget |
                                      rhi::ImageUsage::Sampled,
                      });
      data.depth =
        builder.write(data.depth, Attachment{.clearValue = ClearValue::One});

      data.normal = builder.create<FrameGraphTexture>(
        "Normal", {
                    .extent = resolution,
                    .format = rhi::PixelFormat::RGBA16F,
                    .usageFlags =
                      rhi::ImageUsage::RenderTarget | rhi::ImageUsage::Sampled,
                  });
      data.normal =
        builder.write(data.normal, Attachment{
                                     .index = 0,
                                     .clearValue = ClearValue::TransparentBlack,
                                   });

      data.emissive = builder.create<FrameGraphTexture>(
        "Emissive", {
                      .extent = resolution,
                      .format = rhi::PixelFormat::RGBA16F,
                      .usageFlags = rhi::ImageUsage::RenderTarget |
                                    rhi::ImageUsage::Sampled,
                    });
      data.emissive = builder.write(
        data.emissive, Attachment{
                         .index = 1,
                         .clearValue = ClearValue::TransparentBlack,
                       });

      data.albedo = builder.create<FrameGraphTexture>(
        "Albedo SpecularWeight", {
                                   .extent = resolution,
                                   .format = rhi::PixelFormat::RGBA8_UNorm,
                                   .usageFlags = rhi::ImageUsage::RenderTarget |
                                                 rhi::ImageUsage::Sampled,
                                 });
      data.albedo =
        builder.write(data.albedo, Attachment{
                                     .index = 2,
                                     .clearValue = ClearValue::TransparentBlack,
                                   });

      data.metallicRoughnessAO = builder.create<FrameGraphTexture>(
        "Metallic Roughness AO", {
                                   .extent = resolution,
                                   .format = rhi::PixelFormat::RGBA8_UNorm,
                                   .usageFlags = rhi::ImageUsage::RenderTarget |
                                                 rhi::ImageUsage::Sampled,
                                 });
      data.metallicRoughnessAO = builder.write(
        data.metallicRoughnessAO, Attachment{
                                    .index = 3,
                                    .clearValue = ClearValue::TransparentBlack,
                                  });

      data.misc = builder.create<FrameGraphTexture>(
        "Misc", {
                  .extent = resolution,
                  .format = rhi::PixelFormat::R8_UNorm,
                  .usageFlags =
                    rhi::ImageUsage::RenderTarget | rhi::ImageUsage::Sampled,
                });
      data.misc =
        builder.write(data.misc, Attachment{
                                   .index = 4,
                                   .clearValue = ClearValue::TransparentBlack,
                                 });
    },
    [this, batches = std::move(batches)](
      const GBufferData &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, kPassName)

      auto &[cb, framebufferInfo, sets] = rc;
      BaseGeometryPassInfo passInfo{
        .depthFormat = rhi::getDepthFormat(*framebufferInfo),
        .colorFormats = rhi::getColorFormats(*framebufferInfo),
      };

      cb.beginRendering(*framebufferInfo);
      for (const auto &batch : batches) {
        if (const auto *pipeline = _getPipeline(adjust(passInfo, batch));
            pipeline) {
          render(rc, *pipeline, batch);
        }
      }
      endRendering(rc);
    });
}

CodePair GBufferPass::buildShaderCode(const rhi::RenderDevice &rd,
                                      const VertexFormat *vertexFormat,
                                      const Material &material) {
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
  code.vert = shaderCodeBuilder.buildFromFile("Mesh.vert");

  // -- FragmentShader:

  shaderCodeBuilder.setDefines(commonDefines);
  addMaterial(shaderCodeBuilder, material, rhi::ShaderType::Fragment,
              offsetAlignment);
  code.frag = shaderCodeBuilder.buildFromFile("GBufferPass.frag");

  return code;
}

//
// (private):
//

rhi::GraphicsPipeline
GBufferPass::_createPipeline(const BaseGeometryPassInfo &passInfo) const {
  auto &rd = getRenderDevice();

  const auto &material = *passInfo.material;
  const auto [vertCode, fragCode] =
    buildShaderCode(rd, passInfo.vertexFormat, material);

  return rhi::GraphicsPipeline::Builder{}
    .setDepthFormat(passInfo.depthFormat)
    .setColorFormats(passInfo.colorFormats)
    .setInputAssembly(passInfo.vertexFormat->getAttributes())
    .setTopology(passInfo.topology)
    .addShader(rhi::ShaderType::Vertex, vertCode)
    .addShader(rhi::ShaderType::Fragment, fragCode)

    .setDepthStencil({
      .depthTest = true,
      .depthWrite = true,
      .depthCompareOp = rhi::CompareOp::LessOrEqual,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = getSurface(material).cullMode,
    })
    .setBlending(0, {.enabled = false})
    .setBlending(1, {.enabled = false})
    .setBlending(2, {.enabled = false})
    .setBlending(3, {.enabled = false})
    .setBlending(4, {.enabled = false})
    .build(rd);
}

} // namespace gfx
