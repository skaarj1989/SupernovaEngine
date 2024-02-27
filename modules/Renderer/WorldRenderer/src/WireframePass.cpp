#include "renderer/WireframePass.hpp"

#include "FrameGraphCommon.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "FrameGraphData/DummyResources.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/Transforms.hpp"
#include "FrameGraphData/Skins.hpp"
#include "FrameGraphData/GBuffer.hpp"

#include "MaterialShader.hpp"
#include "BatchBuilder.hpp"
#include "UploadInstances.hpp"

#include "RenderContext.hpp"

namespace gfx {

namespace {

[[nodiscard]] auto batchCompatible(const Batch &b, const Renderable &r) {
  return sameGeometry(b, r) && sameMaterial(b, r);
}

} // namespace

//
// WireframePass class:
//

WireframePass::WireframePass(rhi::RenderDevice &rd)
    : rhi::RenderPass<WireframePass>{rd} {}

uint32_t WireframePass::count(PipelineGroups flags) const {
  return bool(flags & PipelineGroups::SurfaceMaterial) ? BasePass::count() : 0;
}
void WireframePass::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::SurfaceMaterial)) BasePass::clear();
}

FrameGraphResource WireframePass::addGeometryPass(
  FrameGraph &fg, const FrameGraphBlackboard &blackboard,
  FrameGraphResource target, const ViewInfo &viewData) {
  constexpr auto kPassName = "WireframePass";
  ZoneScopedN(kPassName);

  sortByMaterial(viewData.visibleRenderables);

  std::vector<GPUInstance> gpuInstances;
  auto batches = buildBatches(gpuInstances, viewData.visibleRenderables, {},
                              batchCompatible);
  if (batches.empty()) return target;

  const auto instances = *uploadInstances(fg, std::move(gpuInstances));

  fg.addCallbackPass(
    kPassName,
    [&blackboard, &target, instances](FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<CameraData>(), PipelineStage::VertexShader);

      const auto &dummyResources = blackboard.get<DummyResourcesData>();
      readInstances(builder, instances, dummyResources);
      read(builder, blackboard.try_get<TransformData>(), dummyResources);
      read(builder, blackboard.try_get<SkinData>(), dummyResources);
      builder.read(blackboard.get<GBufferData>().depth, Attachment{});

      target = builder.write(target, Attachment{.index = 0});
    },
    [this, batches = std::move(batches)](
      const auto &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      BaseGeometryPassInfo passInfo{
        .depthFormat = rhi::getDepthFormat(*framebufferInfo),
        .colorFormats = rhi::getColorFormats(*framebufferInfo),
      };

      cb.beginRendering(*framebufferInfo);
      for (const auto &batch : batches) {
        if (const auto *pipeline = _getPipeline(adjust(passInfo, batch));
            pipeline) {
          cb.bindPipeline(*pipeline);
          bindDescriptorSets(rc, *pipeline);
          static const glm::vec4 kLineColor{glm::vec3{1.0f}, 0.3f};
          cb.pushConstants(rhi::ShaderStages::Fragment, 16, &kLineColor);
          drawBatch(rc, batch);
        }
      }
      endRendering(rc);
    });

  return target;
}

//
// (private):
//

rhi::GraphicsPipeline
WireframePass::_createPipeline(const BaseGeometryPassInfo &passInfo) const {
  assert(passInfo.vertexFormat && passInfo.material);

  const auto commonDefines = buildDefines(*passInfo.vertexFormat);

  ShaderCodeBuilder shaderCodeBuilder;

  // -- VertexShader:

  shaderCodeBuilder.setDefines(commonDefines);
  setReferenceFrames(shaderCodeBuilder, {
                                          .position = ReferenceFrame::Clip,
                                          .normal = ReferenceFrame::World,
                                        });
  noMaterial(shaderCodeBuilder);
  const auto vert = shaderCodeBuilder.buildFromFile("Mesh.vert");

  // -- FragmentShader:

  const auto frag =
    shaderCodeBuilder.clearDefines().buildFromFile("FlatColor.frag");

  return rhi::GraphicsPipeline::Builder{}
    .setDepthFormat(passInfo.depthFormat)
    .setColorFormats(passInfo.colorFormats)
    .setInputAssembly(passInfo.vertexFormat->getAttributes())
    .setTopology(passInfo.topology)
    .addShader(rhi::ShaderType::Vertex, vert)
    .addShader(rhi::ShaderType::Fragment, frag)

    .setDepthStencil({
      .depthTest = true,
      .depthWrite = false,
      .depthCompareOp = rhi::CompareOp::LessOrEqual,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Line,
      .cullMode = getSurface(*passInfo.material).cullMode,
      .depthBias =
        rhi::DepthBias{
          .constantFactor = -10.0f,
          .slopeFactor = 0.0,
        },
      .lineWidth = 2.0f,
    })
    // clang-format off
    .setBlending(0, {
      .enabled = true,
      .srcColor = rhi::BlendFactor::SrcAlpha,
      .dstColor = rhi::BlendFactor::OneMinusSrcAlpha,
      .colorOp = rhi::BlendOp::Add,

      .srcAlpha = rhi::BlendFactor::One,
      .dstAlpha = rhi::BlendFactor::One,
      .alphaOp = rhi::BlendOp::Add,
    })
    // clang-format on
    .build(getRenderDevice());
}

} // namespace gfx
