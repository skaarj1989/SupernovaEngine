#include "NuklearRenderer.hpp"
#include "ShaderCodeBuilder.hpp"

#define USE_PROJECTION_MATRIX 0
#if USE_PROJECTION_MATRIX
#  include "glm/ext/matrix_float4x4.hpp"
#endif

#define USE_MONOCHROMATIC_FONT 0

#pragma warning(disable : 26812) // Silence "unscoped enum"

namespace {

struct Vertex {
  float position[2];
  float uv[2];
  nk_byte col[4];
};

[[nodiscard]] auto createConfig() {
  static const nk_draw_vertex_layout_element kVertexLayout[]{
    {
      .attribute = NK_VERTEX_POSITION,
      .format = NK_FORMAT_FLOAT,
      .offset = 0,
    },
    {
      .attribute = NK_VERTEX_TEXCOORD,
      .format = NK_FORMAT_FLOAT,
      .offset = NK_OFFSETOF(Vertex, uv),
    },
    {
      .attribute = NK_VERTEX_COLOR,
      .format = NK_FORMAT_R8G8B8A8,
      .offset = NK_OFFSETOF(Vertex, col),
    },
    {NK_VERTEX_LAYOUT_END},
  };
  return nk_convert_config{
    .global_alpha = 1.0f,
    .line_AA = NK_ANTI_ALIASING_ON,
    .shape_AA = NK_ANTI_ALIASING_ON,
    .circle_segment_count = 22,
    .arc_segment_count = 22,
    .curve_segment_count = 22,
    .vertex_layout = kVertexLayout,
    .vertex_size = sizeof(Vertex),
    .vertex_alignment = NK_ALIGNOF(Vertex),
  };
}

[[nodiscard]] auto createFont(rhi::RenderDevice &rd, nk_font_atlas &atlas) {
  int32_t width;
  int32_t height;
#if USE_MONOCHROMATIC_FONT
  const auto kPixelFormat = rhi::PixelFormat::R8_UNorm;
  const void *pixels =
    nk_font_atlas_bake(&atlas, &width, &height, NK_FONT_ATLAS_ALPHA8);
  constexpr auto kPixelSize = 1;
#else
  const auto kPixelFormat = rhi::PixelFormat::RGBA8_UNorm;
  const void *pixels =
    nk_font_atlas_bake(&atlas, &width, &height, NK_FONT_ATLAS_RGBA32);
  constexpr auto kPixelSize = 4;
#endif
  const auto dataSize = width * height * kPixelSize * sizeof(uint8_t);

  auto stagingBuffer = rd.createStagingBuffer(dataSize, pixels);

  auto font =
    rhi::Texture::Builder{}
      .setExtent({uint32_t(width), uint32_t(height)})
      .setPixelFormat(kPixelFormat)
      .setNumMipLevels(1)
      .setNumLayers(std::nullopt)
      .setUsageFlags(rhi::ImageUsage::TransferDst | rhi::ImageUsage::Sampled)
      .build(rd);

  rd.execute([&stagingBuffer, &font](auto &cb) {
      cb.copyBuffer(stagingBuffer, font);
      rhi::prepareForReading(cb, font);
    })
    .setupSampler(font, {
                          .magFilter = rhi::TexelFilter::Linear,
                          .minFilter = rhi::TexelFilter::Linear,
                          .mipmapMode = rhi::MipmapMode::Nearest,
                          .addressModeS = rhi::SamplerAddressMode::Repeat,
                          .addressModeT = rhi::SamplerAddressMode::Repeat,
                          .addressModeR = rhi::SamplerAddressMode::Repeat,
                          .minLod = 0.0f,
                          .maxLod = 1.0f,
                        });
  return font;
}
[[nodiscard]] auto createGraphicsPipeline(rhi::RenderDevice &rd) {
  ShaderCodeBuilder shaderCodeBuilder;
  const auto vertCode =
    shaderCodeBuilder.addDefine("USE_PROJECTION_MATRIX", USE_PROJECTION_MATRIX)
      .buildFromFile("UI.vert");
  const auto fragCode =
    shaderCodeBuilder.clearDefines()
      .addDefine("USE_MONOCHROMATIC_FONT", USE_MONOCHROMATIC_FONT)
      .buildFromFile("UI.frag");

  // clang-format off
  return rhi::GraphicsPipeline::Builder{}
    .setColorFormats({rhi::PixelFormat::BGRA8_UNorm})
    .setInputAssembly({
      {0,
        {
          .type = rhi::VertexAttribute::Type::Float2,
          .offset = 0,
        }},
      {1,
        {
          .type = rhi::VertexAttribute::Type::Float2,
          .offset = offsetof(Vertex, uv),
        }},
      {2,
        {
          .type = rhi::VertexAttribute::Type::UByte4_Norm,
          .offset = offsetof(Vertex, col),
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
      .cullMode = rhi::CullMode::None,
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
    .build(rd);
  // clang-format on
}

#if USE_PROJECTION_MATRIX
[[nodiscard]] glm::mat4 buildProjectionMatrix(const rhi::Extent2D extent) {
  const auto L = 0.0f;
  const auto R = float(extent.width);
  auto B = float(extent.height);
  auto T = 0.0f;
  std::swap(T, B);

  // clang-format off
  return {
    { 2.0f/(R-L),   0.0f,         0.0f,  0.0f },
    { 0.0f,         2.0f/(T-B),   0.0f,  0.0f },
    { 0.0f,         0.0f,        -1.0f,  0.0f },
    { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,  1.0f },
  };
  // clang-format on
}
#endif

} // namespace

//
// NuklearRenderer class:
//

NuklearRenderer::NuklearRenderer(rhi::RenderDevice &rd, nk_context &ctx,
                                 nk_font_atlas &atlas)
    : m_ctx{ctx}, m_config{createConfig()}, m_renderDevice{rd},
      m_font{createFont(rd, atlas)},
      m_graphicsPipeline{createGraphicsPipeline(rd)} {
  nk_font_atlas_end(&atlas, nk_handle_ptr(std::addressof(m_font)),
                    &m_config.tex_null);
  nk_style_set_font(&m_ctx, &atlas.default_font->handle);

  nk_buffer_init_default(&m_commands);
  nk_buffer_init_default(&m_vertices);
  nk_buffer_init_default(&m_indices);
}
NuklearRenderer::~NuklearRenderer() {
  nk_buffer_free(&m_indices);
  nk_buffer_free(&m_vertices);
  nk_buffer_free(&m_commands);
}

std::vector<NuklearRenderer::FrameResources>
NuklearRenderer::createResources(int32_t numFrames) const {
  assert(numFrames > 0);
  std::vector<NuklearRenderer::FrameResources> resources;
  resources.reserve(numFrames);
  std::generate_n(
    std::back_inserter(resources), numFrames,
    std::bind_front(&NuklearRenderer::createFrameResources, this));
  return resources;
}
NuklearRenderer::FrameResources NuklearRenderer::createFrameResources() const {
  return {
    m_renderDevice.createVertexBuffer(sizeof(Vertex), UINT16_MAX,
                                      rhi::AllocationHints::SequentialWrite),
    m_renderDevice.createIndexBuffer(rhi::IndexType::UInt16, UINT16_MAX,
                                     rhi::AllocationHints::SequentialWrite),
  };
}

void NuklearRenderer::draw(rhi::CommandBuffer &cb, FrameResources &resources,
                           rhi::Extent2D extent, glm::vec2 scale) {
  NAMED_DEBUG_MARKER(cb, "Nuklear");
  TracyVkZone(cb.getTracyContext(), cb.getHandle(), "Nuklear");

  nk_buffer_clear(&m_vertices);
  nk_buffer_clear(&m_indices);
  nk_buffer_clear(&m_commands);
  nk_convert(&m_ctx, &m_commands, &m_vertices, &m_indices, &m_config);

  _uploadGeometry(resources);
  _setupRenderState(cb, extent);

  auto descriptorSetBuilder = cb.createDescriptorSetBuilder();

  uint32_t offset{0};
  const struct nk_draw_command *cmd;
  nk_draw_foreach(cmd, &m_ctx, &m_commands) {
    if (cmd->elem_count == 0) continue;

    const auto texture = static_cast<const rhi::Texture *>(cmd->texture.ptr);
    if (!texture || !*texture) continue;

    rhi::Rect2D scissor{
      .offset =
        {
          .x = int32_t(cmd->clip_rect.x * scale.x),
          .y = int32_t(cmd->clip_rect.y * scale.y),
        },
      .extent =
        {
          .width = uint32_t(cmd->clip_rect.w * scale.x),
          .height = uint32_t(cmd->clip_rect.h * scale.y),
        },
    };
    // Clamp to viewport, vkCmdSetScissor() won't accept values off bounds.
    if (scissor.offset.x < 0) scissor.offset.x = 0;
    if (scissor.offset.y < 0) scissor.offset.y = 0;
    if (scissor.extent.width > extent.width)
      scissor.extent.width = extent.width;
    if (scissor.extent.height > extent.height)
      scissor.extent.height = extent.height;

    constexpr auto kDescriptorSetId = 0;

    auto descriptorSet =
      descriptorSetBuilder.bind(0, rhi::bindings::CombinedImageSampler{texture})
        .build(m_graphicsPipeline.getDescriptorSetLayout(kDescriptorSetId));

    cb.bindDescriptorSet(kDescriptorSetId, descriptorSet)
      .setScissor(scissor)
      .draw({
        .vertexBuffer = &resources.vertexBuffer,
        .vertexOffset = 0,
        .indexBuffer = &resources.indexBuffer,
        .indexOffset = offset,
        .numIndices = cmd->elem_count,
      });
    offset += cmd->elem_count;
  }
}

//
// (private):
//

void NuklearRenderer::_uploadGeometry(FrameResources &resources) const {
  auto &[vertexBuffer, indexBuffer] = resources;

  auto *vertexDest = static_cast<Vertex *>(vertexBuffer.map());
  std::memcpy(vertexDest, m_vertices.memory.ptr, m_vertices.size);

  auto *indexDest = static_cast<uint16_t *>(indexBuffer.map());
  std::memcpy(indexDest, m_indices.memory.ptr, m_indices.size);

  vertexBuffer.flush();
  indexBuffer.flush();
}
void NuklearRenderer::_setupRenderState(rhi::CommandBuffer &cb,
                                        rhi::Extent2D extent) const {
#if USE_PROJECTION_MATRIX
  const auto projectionMatrix = buildProjectionMatrix(extent);
#else
  struct Uniforms {
    Uniforms(glm::vec2 pos, glm::vec2 size)
        : scale{2.0f / size.x, 2.0f / size.y},
          translate{-1.0f - pos.x * scale.x, -1.0f - pos.y * scale.y} {}

    glm::vec2 scale;
    glm::vec2 translate;
  };
  const Uniforms uniforms{{0, 0}, {extent.width, extent.height}};
#endif

  cb.bindPipeline(m_graphicsPipeline)
#if USE_PROJECTION_MATRIX
    .pushConstants(rhi::ShaderStages::Vertex, 0, &projectionMatrix);
#else
    .pushConstants(rhi::ShaderStages::Vertex, 0, &uniforms);
#endif
}
