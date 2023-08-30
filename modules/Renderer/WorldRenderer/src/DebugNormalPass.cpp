#include "renderer/DebugNormalPass.hpp"

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

DebugNormalPass::DebugNormalPass(rhi::RenderDevice &rd)
    : rhi::RenderPass<DebugNormalPass>{rd} {}

uint32_t DebugNormalPass::count(PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void DebugNormalPass::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource DebugNormalPass::addGeometryPass(
  FrameGraph &fg, const FrameGraphBlackboard &blackboard,
  FrameGraphResource target, const ViewInfo &viewData) {
  ZoneScoped;

  constexpr auto kPassName = "DebugNormalPass";

  std::vector<GPUInstance> gpuInstances;
  auto batches =
    buildBatches(gpuInstances, viewData.visibleRenderables, {}, sameGeometry);
  if (batches.empty()) return target;

  const auto instances = uploadInstances(fg, std::move(gpuInstances));

  fg.addCallbackPass(
    kPassName,
    [&blackboard, &target, instances](FrameGraph::Builder &builder, auto &) {
      read(builder, blackboard.get<CameraData>(),
           PipelineStage::VertexShader | PipelineStage::GeometryShader);

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
      ZONE(rc, kPassName)

      auto &[cb, framebufferInfo, sets] = rc;
      BaseGeometryPassInfo passInfo{
        .depthFormat = rhi::getDepthFormat(*framebufferInfo),
        .colorFormats = rhi::getColorFormats(*framebufferInfo),
      };

      cb.beginRendering(*framebufferInfo);
      for (const auto &batch : batches) {
        const auto *pipeline = _getPipeline(adjust(passInfo, batch));
        if (pipeline) {
          cb.bindPipeline(*pipeline);
          bindDescriptorSets(rc, *pipeline);
          static const glm::vec4 kLineColor{0.156f, 0.563f, 1.0f, 1.0f};
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
DebugNormalPass::_createPipeline(const BaseGeometryPassInfo &passInfo) const {
  const auto commonDefines = buildDefines(*passInfo.vertexFormat);

  ShaderCodeBuilder shaderCodeBuilder;
  shaderCodeBuilder.setDefines(commonDefines);
  setReferenceFrames(shaderCodeBuilder, {
                                          .position = ReferenceFrame::View,
                                          .normal = ReferenceFrame::View,
                                        });
  noMaterial(shaderCodeBuilder);
  const auto vert = shaderCodeBuilder.buildFromFile("Mesh.vert");
  const auto geom = shaderCodeBuilder.setDefines(commonDefines)
                      .buildFromFile("DebugNormal.geom");
  const auto frag =
    shaderCodeBuilder.clearDefines().buildFromFile("FlatColor.frag");

  return rhi::GraphicsPipeline::Builder{}
    .setDepthFormat(passInfo.depthFormat)
    .setColorFormats(passInfo.colorFormats)
    .setInputAssembly(passInfo.vertexFormat->getAttributes())
    .setTopology(passInfo.topology)
    .addShader(rhi::ShaderType::Vertex, vert)
    .addShader(rhi::ShaderType::Geometry, geom)
    .addShader(rhi::ShaderType::Fragment, frag)
    .setDepthStencil({
      .depthTest = true,
      .depthWrite = false,
      .depthCompareOp = rhi::CompareOp::LessOrEqual,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Back,
      .lineWidth = 2.0f,
    })
    .setBlending(0, {.enabled = false})
    .build(getRenderDevice());
}

} // namespace gfx
