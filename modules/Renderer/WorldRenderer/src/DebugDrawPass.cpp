#include "renderer/DebugDrawPass.hpp"

#include "MergeVector.hpp"
#include "DebugDraw.hpp"

#include "rhi/CommandBuffer.hpp"
#include "rhi/VertexBuffer.hpp"
#include "rhi/IndexBuffer.hpp"

#include "FrameGraphCommon.hpp"
#include "UploadContainer.hpp"

#include "FrameGraphData/DummyResources.hpp"
#include "FrameGraphData/Camera.hpp"
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

struct PrimitiveInfo {
  rhi::PrimitiveTopology topology;
  DebugDraw::Range range;
};
using PrimitiveInfoList = std::array<PrimitiveInfo, 2>;

[[nodiscard]] auto mergeVertices(const DebugDraw::Primitives &primitives) {
  ZoneScopedN("MergeDebugVertices");

  const auto &[points, lines, meshes] = primitives;
  const auto &triangles = meshes.registry ? meshes.registry->getVertexList()
                                          : DebugDraw::VertexList{};
  return std::pair{
    merge(triangles, points, lines),
    PrimitiveInfoList{
      PrimitiveInfo{
        .topology = rhi::PrimitiveTopology::PointList,
        .range =
          {
            .offset = uint32_t(triangles.size()),
            .count = uint32_t(points.size()),
          },
      },
      PrimitiveInfo{
        .topology = rhi::PrimitiveTopology::LineList,
        .range =
          {
            .offset = uint32_t(triangles.size() + points.size()),
            .count = uint32_t(lines.size()),
          },
      },
    },
  };
}

struct Buffers {
  std::optional<FrameGraphResource> vertices;
  std::optional<FrameGraphResource> indices;
  std::optional<FrameGraphResource> instances;
};
[[nodiscard]] auto uploadBuffers(FrameGraph &fg,
                                 const DebugDraw::Primitives &primitives) {
  ZoneScopedN("UploadDebugBuffers");

  auto [vertices, info] = mergeVertices(primitives);
  return std::pair{
    Buffers{
      .vertices = uploadContainer(fg, "UploadDebugVertices",
                                  TransientBuffer{
                                    .name = "VertexBuffer",
                                    .type = BufferType::VertexBuffer,
                                    .data = std::move(vertices),
                                  }),
      .indices =
        uploadContainer(fg, "UploadDebugTriangles(indices)",
                        TransientBuffer{
                          .name = "IndexBuffer",
                          .type = BufferType::IndexBuffer,
                          .data = primitives.meshes.registry
                                    ? primitives.meshes.registry->getIndexList()
                                    : DebugDraw::IndexList{},
                        }),
      .instances = uploadContainer(fg, "UploadDebugTriangles(instances)",
                                   TransientBuffer{
                                     .name = "InstanceBuffer",
                                     .type = BufferType::StorageBuffer,
                                     .data = primitives.meshes.instances,
                                   }),
    },
    info,
  };
}

} // namespace

//
// DebugDrawPass class:
//

DebugDrawPass::DebugDrawPass(rhi::RenderDevice &rd)
    : rhi::RenderPass<DebugDrawPass>{rd} {}

uint32_t DebugDrawPass::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void DebugDrawPass::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource DebugDrawPass::addGeometryPass(
  FrameGraph &fg, const FrameGraphBlackboard &blackboard,
  FrameGraphResource target, DebugDraw &debugDraw) {
  assert(!debugDraw.empty());

  static constexpr auto kPassName = "DebugDraw";
  ZoneScopedN(kPassName);

  const auto &primitives = debugDraw.getPrimitives();
  const auto [buffers, info] = uploadBuffers(fg, primitives);

  fg.addCallbackPass(
    kPassName,
    [&blackboard, &target, buffers](FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      if (buffers.vertices) {
        builder.read(*buffers.vertices,
                     BindingInfo{.pipelineStage = PipelineStage::VertexShader});
      }
      if (buffers.indices) {
        builder.read(*buffers.indices,
                     BindingInfo{.pipelineStage = PipelineStage::VertexShader});
      }
      read(builder, blackboard.get<CameraData>(), PipelineStage::VertexShader);
      readInstances(builder, buffers.instances,
                    blackboard.get<DummyResourcesData>());

      builder.read(blackboard.get<GBufferData>().depth,
                   Attachment{.imageAspect = rhi::ImageAspect::Depth});
      target = builder.write(target, Attachment{
                                       .index = 0,
                                       .imageAspect = rhi::ImageAspect::Color,
                                     });
    },
    [this, buffers, info, drawCalls = primitives.meshes.drawInfo](
      const auto &, FrameGraphPassResources &resources, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, _, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      const auto *vertexBuffer =
        buffers.vertices
          ? static_cast<const rhi::VertexBuffer *>(
              resources.get<FrameGraphBuffer>(*buffers.vertices).buffer)
          : nullptr;
      const auto *indexBuffer =
        buffers.indices
          ? static_cast<const rhi::IndexBuffer *>(
              resources.get<FrameGraphBuffer>(*buffers.indices).buffer)
          : nullptr;

      const auto depthFormat = rhi::getDepthFormat(*framebufferInfo);
      const auto colorFormat = rhi::getColorFormat(*framebufferInfo, 0);

      cb.beginRendering(*framebufferInfo);
      if (!drawCalls.empty()) {
        const auto *pipeline = _getPipeline(PassInfo{
          .depthFormat = depthFormat,
          .colorFormat = colorFormat,
          .primitiveTopology = rhi::PrimitiveTopology::TriangleList,
        });
        if (pipeline) {
          cb.bindPipeline(*pipeline);
          bindDescriptorSets(rc, *pipeline);
          for (const auto &[mesh, instanceRange] : drawCalls) {
            cb.pushConstants(rhi::ShaderStages::Vertex, 0,
                             &instanceRange.offset)
              .draw(
                {
                  .topology = rhi::PrimitiveTopology::TriangleList,
                  .vertexBuffer = vertexBuffer,
                  .vertexOffset = mesh->vertices.offset,
                  .numVertices = mesh->vertices.count,
                  .indexBuffer = indexBuffer,
                  .indexOffset = mesh->indices.offset,
                  .numIndices = mesh->indices.count,
                },
                instanceRange.count);
          }
        }
      }

      for (const auto &[topology, range] : info) {
        if (const auto [offset, count] = range; count > 0) {
          const auto *pipeline = _getPipeline(PassInfo{
            .depthFormat = depthFormat,
            .colorFormat = colorFormat,
            .primitiveTopology = topology,
          });
          if (pipeline) {
            cb.bindPipeline(*pipeline);
            bindDescriptorSets(rc, *pipeline);
            cb.draw({
              .topology = topology,
              .vertexBuffer = vertexBuffer,
              .vertexOffset = offset,
              .numVertices = count,
            });
          }
        }
      }
      endRendering(rc);
    });

  debugDraw.clear();

  return target;
}

//
// (private):
//

rhi::GraphicsPipeline
DebugDrawPass::_createPipeline(const PassInfo &passInfo) const {
  ShaderCodeBuilder shaderCodeBuilder{};
  if (passInfo.primitiveTopology == rhi::PrimitiveTopology::TriangleList) {
    shaderCodeBuilder.addDefine("TRIANGLE_MESH", 1);
  }

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
               shaderCodeBuilder.clearDefines().buildFromFile("DebugDraw.frag"))

    .setDepthStencil({
      .depthTest = true,
      .depthWrite = false,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Line,
      .cullMode = rhi::CullMode::Back,
      .lineWidth = 1.6f,
    })
    .setBlending(0, {.enabled = false})
    .build(getRenderDevice());
}

} // namespace gfx
