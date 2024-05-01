#include "renderer/OutlineRenderer.hpp"
#include "rhi/RenderDevice.hpp"

#include "renderer/ViewInfo.hpp"
#include "renderer/VertexFormat.hpp"
#include "renderer/MeshInstance.hpp"

#include "renderer/PostProcess.hpp"

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"
#include "FrameGraphCommon.hpp"
#include "FrameGraphResourceAccess.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "UploadInstances.hpp"

#include "FrameGraphData/DummyResources.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/Transforms.hpp"
#include "FrameGraphData/Skins.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/Silhouette.hpp"

#include "MaterialShader.hpp"
#include "BatchBuilder.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

namespace gfx {

//
// OutlineRenderer class:
//

OutlineRenderer::OutlineRenderer(rhi::RenderDevice &rd)
    : m_silhouettePass{rd}, m_outlinePass{rd} {}

uint32_t OutlineRenderer::count(const PipelineGroups flags) const {
  return m_silhouettePass.count(flags) + m_outlinePass.count(flags);
}
void OutlineRenderer::clear(const PipelineGroups flags) {
  m_silhouettePass.clear(flags);
  m_outlinePass.clear(flags);
}

FrameGraphResource
OutlineRenderer::addOutlines(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                             const ViewInfo &viewInfo,
                             const FrameGraphResource sceneColor) {
  const auto extent = fg.getDescriptor<FrameGraphTexture>(sceneColor).extent;
  m_silhouettePass.addGeometryPass(fg, blackboard, extent, viewInfo);
  return m_outlinePass.addPostProcessPass(fg, blackboard, sceneColor);
}

//
// StencilPass class:
//

OutlineRenderer::SilhouettePass::SilhouettePass(rhi::RenderDevice &rd)
    : rhi::RenderPass<OutlineRenderer::SilhouettePass>{rd} {}

void OutlineRenderer::SilhouettePass::addGeometryPass(
  FrameGraph &fg, FrameGraphBlackboard &blackboard,
  const rhi::Extent2D resolution, const ViewInfo &viewInfo) {
  static constexpr auto kPassName = "SilhouettePass";
  ZoneScopedN(kPassName);

  std::vector<const Renderable *> renderables;
  renderables.reserve(viewInfo.visibleRenderables.size());
  std::ranges::copy_if(viewInfo.visibleRenderables,
                       std::back_inserter(renderables),
                       [](const Renderable *renderable) {
                         return bool(renderable->subMeshInstance.flags &
                                     SubMeshInstance::Flags::ShowOutline);
                       });
  if (renderables.empty()) return;

  std::vector<GPUInstance> gpuInstances;
  auto batches = buildBatches(gpuInstances, renderables, {}, sameGeometry);

  const auto instances = uploadInstances(fg, std::move(gpuInstances));

  blackboard.add<SilhouetteData>() = fg.addCallbackPass<SilhouetteData>(
    kPassName,
    [&fg, &blackboard, resolution, instances](FrameGraph::Builder &builder,
                                              SilhouetteData &data) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<CameraData>());

      const auto &dummyResources = blackboard.get<DummyResourcesData>();
      readInstances(builder, instances, dummyResources);
      read(builder, blackboard.try_get<TransformData>(), dummyResources);
      read(builder, blackboard.try_get<SkinData>(), dummyResources);

      data.depthStencil = builder.create<FrameGraphTexture>(
        "SilhouetteBuffer", {
                              .extent = resolution,
                              .format = rhi::PixelFormat::Depth32F_Stencil8,
                              .usageFlags = rhi::ImageUsage::RenderTarget |
                                            rhi::ImageUsage::Sampled,
                            });
      data.depthStencil = builder.write(
        data.depthStencil, Attachment{
                             .imageAspect = rhi::ImageAspect::Depth,
                             .clearValue = ClearValue::One,
                           });
      data.depthStencil = builder.write(
        data.depthStencil, Attachment{
                             .imageAspect = rhi::ImageAspect::Stencil,
                             .clearValue = ClearValue::Zero,
                           });
    },
    [this, batches = std::move(batches)](
      const SilhouetteData &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, _, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      BaseGeometryPassInfo passInfo{
        .depthFormat = rhi::getDepthFormat(*framebufferInfo),
      };
      cb.beginRendering(*framebufferInfo);
      for (const auto &batch : batches) {
        if (const auto *pipeline = _getPipeline(adjust(passInfo, batch));
            pipeline) {
          cb.bindPipeline(*pipeline);
          bindDescriptorSets(rc, *pipeline);
          drawBatch(rc, batch);
        }
      }
      endRendering(rc);
    });
}

uint32_t
OutlineRenderer::SilhouettePass::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::SurfaceMaterial) ? BasePass::count() : 0;
}
void OutlineRenderer::SilhouettePass::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::SurfaceMaterial)) BasePass::clear();
}

//
// (private):
//

rhi::GraphicsPipeline OutlineRenderer::SilhouettePass::_createPipeline(
  const BaseGeometryPassInfo &passInfo) const {
  ShaderCodeBuilder shaderCodeBuilder;
  shaderCodeBuilder.setDefines(buildDefines(*passInfo.vertexFormat));
  setReferenceFrames(shaderCodeBuilder, {
                                          .position = ReferenceFrame::Clip,
                                          .normal = ReferenceFrame::World,
                                        });
  noMaterial(shaderCodeBuilder);
  const auto vertCode = shaderCodeBuilder.buildFromFile("Mesh.vert");
  const auto fragCode = "#version 460\nvoid main() {}";

  return rhi::GraphicsPipeline::Builder{}
    .setDepthFormat(passInfo.depthFormat)
    .setInputAssembly(passInfo.vertexFormat->getAttributes())
    .setTopology(passInfo.topology)
    .addShader(rhi::ShaderType::Vertex, vertCode)
    .addShader(rhi::ShaderType::Fragment, fragCode)

    .setDepthStencil({
      .depthTest = true,
      .depthWrite = true,
      .depthCompareOp = rhi::CompareOp::LessOrEqual,
      .stencilTestEnable = true,
      .front =
        {
          .failOp = rhi::StencilOp::Replace,
          .passOp = rhi::StencilOp::Replace,
          .depthFailOp = rhi::StencilOp::Replace,
          .compareOp = rhi::CompareOp::Always,
          .compareMask = 0xFF,
          .writeMask = 0xFF,
          .reference = 1,
        },
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Back,
    })
    .build(getRenderDevice());
}

//
// OutlinePass class:
//

OutlineRenderer::OutlinePass::OutlinePass(rhi::RenderDevice &rd)
    : rhi::RenderPass<OutlineRenderer::OutlinePass>{rd} {}

uint32_t OutlineRenderer::OutlinePass::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void OutlineRenderer::OutlinePass::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource OutlineRenderer::OutlinePass::addPostProcessPass(
  FrameGraph &fg, const FrameGraphBlackboard &blackboard,
  FrameGraphResource target) {
  static constexpr auto kPassName = "OutlinePostProcess";
  ZoneScopedN(kPassName);

  const auto *silhouetteData = blackboard.try_get<SilhouetteData>();
  if (!silhouetteData) return target;

  fg.addCallbackPass(
    kPassName,
    [&fg, &blackboard, &target, depthStencil = silhouetteData->depthStencil](
      FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<CameraData>(),
           PipelineStage::FragmentShader);

      builder.read(depthStencil,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 2, .binding = 0},
                         .pipelineStage = PipelineStage::FragmentShader,
                       },
                     .type = TextureRead::Type::CombinedImageSampler,
                     .imageAspect = rhi::ImageAspect::Depth,
                   });
      builder.read(depthStencil,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 2, .binding = 1},
                         .pipelineStage = PipelineStage::FragmentShader,
                       },
                     .type = TextureRead::Type::CombinedImageSampler,
                     .imageAspect = rhi::ImageAspect::Stencil,
                   });
      builder.read(blackboard.get<GBufferData>().depth,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 2, .binding = 2},
                         .pipelineStage = PipelineStage::FragmentShader,
                       },
                     .type = TextureRead::Type::CombinedImageSampler,
                     .imageAspect = rhi::ImageAspect::Depth,
                   });
      target = builder.write(target, Attachment{
                                       .index = 0,
                                       .imageAspect = rhi::ImageAspect::Color,
                                     });
    },
    [this](const auto &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, commonSamplers, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      if (const auto *pipeline =
            _getPipeline(rhi::getColorFormat(*framebufferInfo, 0));
          pipeline) {
        for (auto i = 0; i < 3; ++i)
          overrideSampler(sets[2][i], commonSamplers.point);

        struct Uniforms {
          glm::vec4 color;
          uint32_t numSteps;
          float radius;
        };
        const Uniforms uniforms{
          .color = glm::vec4{0.920f, 0.180f, 0.290f, 0.760f},
          .numSteps = 8,
          .radius = 3.0f,
        };
        cb.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);
        cb.pushConstants(rhi::ShaderStages::Fragment, 0, &uniforms)
          .beginRendering(*rc.framebufferInfo)
          .drawFullScreenTriangle();
        endRendering(rc);
      }
    });
  return target;
}

//
// (private):
//

rhi::GraphicsPipeline OutlineRenderer::OutlinePass::_createPipeline(
  const rhi::PixelFormat colorFormat) const {
  ShaderCodeBuilder shaderCodeBuilder;

  return rhi::GraphicsPipeline::Builder{}
    .setColorFormats({colorFormat})
    .setInputAssembly({})
    .addShader(rhi::ShaderType::Vertex,
               shaderCodeBuilder.buildFromFile("FullScreenTriangle.vert"))
    .addShader(rhi::ShaderType::Fragment,
               shaderCodeBuilder.buildFromFile("OutlinePass.frag"))

    .setDepthStencil({
      .depthTest = false,
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
