#include "renderer/DebugDrawPass.hpp"
#include "MergeVector.hpp"
#include "UploadContainer.hpp"
#include "fg/Blackboard.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "ShaderCodeBuilder.hpp"

namespace std {

template <> struct hash<gfx::DebugDrawPass::PassInfo> {
  auto operator()(const gfx::DebugDrawPass::PassInfo &passInfo) const noexcept {
    size_t h{0};
    hashCombine(h, passInfo.depthFormat, passInfo.colorFormat,
                passInfo.primitiveTopology);
    return h;
  }
};

} // namespace std

namespace gfx {

namespace {

struct Info {
  uint32_t numPoints;
  uint32_t numLineVerts;
};
[[nodiscard]] auto mergeVertices(const DebugDraw &debugDraw) {
  const auto &[points, lines] = debugDraw.getPrimitives();
  return std::pair{
    merge(points, lines),
    Info{
      .numPoints = uint32_t(points.size()),
      .numLineVerts = uint32_t(lines.size()),
    },
  };
}

[[nodiscard]] auto uploadVertices(FrameGraph &fg,
                                  DebugDraw::Primitives::Vertices &&vertices) {
  return uploadContainer(fg, "UploadDebugVertices",
                         TransientBuffer{
                           .name = "VertexBuffer",
                           .type = BufferType::VertexBuffer,
                           .data = std::move(vertices),
                         });
}

} // namespace

//
// DebugDrawPass class:
//

DebugDrawPass::DebugDrawPass(rhi::RenderDevice &rd)
    : rhi::RenderPass<DebugDrawPass>{rd} {}

uint32_t DebugDrawPass::count(PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void DebugDrawPass::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource
DebugDrawPass::addGeometryPass(FrameGraph &fg,
                               const FrameGraphBlackboard &blackboard,
                               FrameGraphResource target, DebugDraw &debugDraw,
                               const glm::mat4 &viewProjection) {
  assert(!debugDraw.empty());
  ZoneScoped;

  constexpr auto kPassName = "DebugDraw";

  auto [buffer, info] = mergeVertices(debugDraw);
  debugDraw.clear();

  const auto vertices = uploadVertices(fg, std::move(buffer));

  fg.addCallbackPass(
    kPassName,
    [&blackboard, &target, vertices](FrameGraph::Builder &builder, auto &) {
      builder.read(vertices,
                   BindingInfo{.pipelineStage = PipelineStage::VertexShader});
      builder.read(blackboard.get<GBufferData>().depth, Attachment{});

      target = builder.write(target, Attachment{.index = 0});
    },
    [this, viewProjection, info,
     vertices](const auto &, FrameGraphPassResources &resources, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, kPassName)

      auto *vertexBuffer = static_cast<const rhi::VertexBuffer *>(
        resources.get<FrameGraphBuffer>(vertices).buffer);

      auto &[cb, framebufferInfo, sets] = rc;
      const auto depthFormat = rhi::getDepthFormat(*framebufferInfo);
      const auto colorFormat = rhi::getColorFormat(*framebufferInfo, 0);

      cb.beginRendering(*framebufferInfo);
      if (info.numPoints > 0) {
        const auto *pipeline = _getPipeline(PassInfo{
          .depthFormat = depthFormat,
          .colorFormat = colorFormat,
          .primitiveTopology = rhi::PrimitiveTopology::PointList,
        });
        if (pipeline) {
          cb.bindPipeline(*pipeline)
            .pushConstants(rhi::ShaderStages::Vertex, 0, &viewProjection)
            .draw({
              .vertexBuffer = vertexBuffer,
              .vertexOffset = 0,
              .numVertices = info.numPoints,
            });
        }
      }
      if (info.numLineVerts > 0) {
        const auto *pipeline = _getPipeline(PassInfo{
          .depthFormat = depthFormat,
          .colorFormat = colorFormat,
          .primitiveTopology = rhi::PrimitiveTopology::LineList,
        });
        if (pipeline) {
          cb.bindPipeline(*pipeline)
            .pushConstants(rhi::ShaderStages::Vertex, 0, &viewProjection)
            .draw({
              .vertexBuffer = vertexBuffer,
              .vertexOffset = info.numPoints,
              .numVertices = info.numLineVerts,
            });
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
DebugDrawPass::_createPipeline(const PassInfo &passInfo) const {
  ShaderCodeBuilder shaderCodeBuilder{};

  return rhi::GraphicsPipeline::Builder{}
    .setDepthFormat(passInfo.depthFormat)
    .setColorFormats({passInfo.colorFormat})
    .setTopology(passInfo.primitiveTopology)
    .setInputAssembly({
      {
        0,
        {
          .type = rhi::VertexAttribute::Type::Float4,
          .offset = 0,
        },
      },
      {
        1,
        {
          .type = rhi::VertexAttribute::Type::UByte4_Norm,
          .offset = offsetof(DebugDraw::Vertex, color),
        },
      },
    })
    .addShader(rhi::ShaderType::Vertex,
               shaderCodeBuilder.buildFromFile("DebugDraw.vert"))
    .addShader(rhi::ShaderType::Fragment,
               shaderCodeBuilder.buildFromFile("DebugDraw.frag"))

    .setDepthStencil({
      .depthTest = true,
      .depthWrite = false,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::None,
      .lineWidth = 1.6f,
    })
    .setBlending(0, {.enabled = false})
    .build(getRenderDevice());
}

} // namespace gfx
