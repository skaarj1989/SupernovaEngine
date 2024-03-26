#include "RmlUiRenderer.hpp"
#include "rhi/RenderDevice.hpp"
#include "ShaderCodeBuilder.hpp"

namespace {

[[nodiscard]] auto uploadGeometry(const RmlUiRenderer::TriangleList &triangles,
                                  RmlUiRenderer::FrameResources &resources) {
  ZoneScopedN("RmlUi::UploadGeometry");

  auto &[vertexBuffer, vertexOffset, indexBuffer, indexOffset] = resources;
  const auto &[vertices, indices] = triangles;

  rhi::GeometryInfo gi{
    .topology = rhi::PrimitiveTopology::TriangleList,

    .vertexBuffer = &vertexBuffer,
    .vertexOffset = vertexOffset,
    .numVertices = uint32_t(vertices.size()),

    .indexBuffer = &indexBuffer,
    .indexOffset = indexOffset,
    .numIndices = uint32_t(indices.size()),
  };

  auto *vertexDest = static_cast<Rml::Vertex *>(vertexBuffer.map());
  vertexDest += gi.vertexOffset;
  std::memcpy(vertexDest, vertices.data(), vertices.size_bytes());
  vertexOffset += gi.numVertices;
  vertexBuffer.flush();

  auto *indexDest = static_cast<int32_t *>(indexBuffer.map());
  indexDest += gi.indexOffset;
  std::memcpy(indexDest, indices.data(), indices.size_bytes());
  indexOffset += gi.numIndices;
  indexBuffer.flush();

  return gi;
}

} // namespace

//
// RmlUiRenderer class:
//

RmlUiRenderer::RmlUiRenderer(rhi::RenderDevice &rd)
    : rhi::RenderPass<RmlUiRenderer>{rd} {}

std::vector<RmlUiRenderer::FrameResources> RmlUiRenderer::createResources(
  const rhi::FrameIndex::ValueType numFrames) const {
  assert(numFrames > 0);
  std::vector<RmlUiRenderer::FrameResources> resources;
  resources.reserve(numFrames);
  std::generate_n(std::back_inserter(resources), numFrames,
                  std::bind_front(&RmlUiRenderer::createFrameResources, this));
  return resources;
}
RmlUiRenderer::FrameResources RmlUiRenderer::createFrameResources() const {
  auto &rd = getRenderDevice();
  return {
    .vertexBuffer = rd.createVertexBuffer(
      sizeof(Rml::Vertex), UINT16_MAX, rhi::AllocationHints::SequentialWrite),
    .indexBuffer = rd.createIndexBuffer(rhi::IndexType::UInt32, UINT16_MAX,
                                        rhi::AllocationHints::SequentialWrite),
  };
}

void RmlUiRenderer::draw(rhi::CommandBuffer &cb,
                         const rhi::PixelFormat colorFormat,
                         FrameResources &resources,
                         const TriangleList &triangles,
                         const rhi::Texture *texture,
                         const Uniforms &uniforms) {
  const auto *pipeline = _getPipeline(colorFormat, texture != nullptr);
  if (!pipeline) return;

  const auto gi = uploadGeometry(triangles, resources);

  cb.bindPipeline(*pipeline);
  if (texture) {
    constexpr auto kDescriptorSetId = 0;
    auto descriptorSet =
      cb.createDescriptorSetBuilder()
        .bind(0, rhi::bindings::CombinedImageSampler{texture})
        .build(pipeline->getDescriptorSetLayout(kDescriptorSetId));
    cb.bindDescriptorSet(kDescriptorSetId, descriptorSet);
  }
  cb.pushConstants(rhi::ShaderStages::Vertex, 0, &uniforms).draw(gi);
}

//
// (private):
//

rhi::GraphicsPipeline
RmlUiRenderer::_createPipeline(const rhi::PixelFormat colorFormat,
                               const bool textured) const {
  ShaderCodeBuilder shaderCodeBuilder{};
  const auto vertCode = shaderCodeBuilder.buildFromFile("RmlUI.vert");

  const auto fragCode =
    shaderCodeBuilder.addDefine<int32_t>("HAS_TEXTURE", textured)
      .buildFromFile("RmlUI.frag");

  // clang-format off
  return rhi::GraphicsPipeline::Builder{}
    .setColorFormats({colorFormat})
    .setInputAssembly({
      {0,
        {
          .type = rhi::VertexAttribute::Type::Float2,
          .offset = 0,
        }},
      {1,
        {
          .type = rhi::VertexAttribute::Type::UByte4_Norm,
          .offset = offsetof(Rml::Vertex, colour),
        }},
      {2,
        {
          .type = rhi::VertexAttribute::Type::Float2,
          .offset = offsetof(Rml::Vertex, tex_coord),
        }},
    })
    .addShader(rhi::ShaderType::Vertex, vertCode)
    .addShader(rhi::ShaderType::Fragment, fragCode)

    .setDepthStencil({
      .depthTest = false,
      .depthWrite = false,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Back,
    })
    .setBlending(0, {
      .enabled = true,

      .srcColor = rhi::BlendFactor::SrcAlpha,
      .dstColor = rhi::BlendFactor::OneMinusSrcAlpha,
      .colorOp = rhi::BlendOp::Add,

      .srcAlpha = rhi::BlendFactor::One,
      .dstAlpha = rhi::BlendFactor::OneMinusSrcAlpha,
      .alphaOp = rhi::BlendOp::Add,
    })
    .build(getRenderDevice());
  // clang-format on
}
