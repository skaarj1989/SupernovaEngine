#include "ImGuiRenderer.hpp"
#include "ShaderCodeBuilder.hpp"
#include "imgui.h"
#include "glm/common.hpp" // min, max

#define USE_PROJECTION_MATRIX 1
#if USE_PROJECTION_MATRIX
#  include "glm/ext/matrix_float4x4.hpp"
#endif

#define USE_MONOCHROMATIC_FONT 0

#pragma warning(disable : 26812) // Silence unscoped enum

namespace {

[[nodiscard]] auto createFont(rhi::RenderDevice &rd) {
  uint8_t *pixels{nullptr};
  int32_t width;
  int32_t height;
#if USE_MONOCHROMATIC_FONT
  const auto kPixelFormat = rhi::PixelFormat::R8_UNorm;
  ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
  constexpr auto kPixelSize = 1;
#else
  const auto kPixelFormat = rhi::PixelFormat::RGBA8_UNorm;
  ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  constexpr auto kPixelSize = 4;
#endif
  const auto dataSize = width * height * kPixelSize * sizeof(decltype(*pixels));

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

#if USE_PROJECTION_MATRIX
[[nodiscard]] glm::mat4 buildProjectionMatrix(const ImVec2 displayPos,
                                              const ImVec2 displaySize) {
  const auto L = displayPos.x;
  const auto R = displayPos.x + displaySize.x;
  auto B = displayPos.y + displaySize.y;
  auto T = displayPos.y;
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

void uploadGeometry(const ImDrawData *drawData,
                    ImGuiRenderer::FrameResources &resources) {
  auto &[vertexBuffer, indexBuffer] = resources;

  auto *vertexDest = static_cast<ImDrawVert *>(vertexBuffer.map());
  auto *indexDest = static_cast<ImDrawIdx *>(indexBuffer.map());

  for (const auto *cmdList : drawData->CmdLists) {
    std::memcpy(vertexDest, cmdList->VtxBuffer.Data,
                sizeof(ImDrawVert) * cmdList->VtxBuffer.Size);
    vertexDest += cmdList->VtxBuffer.Size;

    std::memcpy(indexDest, cmdList->IdxBuffer.Data,
                sizeof(ImDrawIdx) * cmdList->IdxBuffer.Size);
    indexDest += cmdList->IdxBuffer.Size;
  }

  vertexBuffer.flush();
  indexBuffer.flush();
}

} // namespace

ImGuiRenderer::ImGuiRenderer(rhi::RenderDevice &rd)
    : rhi::RenderPass<ImGuiRenderer>{rd}, m_font{createFont(rd)} {
  auto &io = ImGui::GetIO();
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
  io.Fonts->SetTexID(&m_font);
}

std::vector<ImGuiRenderer::FrameResources> ImGuiRenderer::createResources(
  const rhi::FrameIndex::ValueType numFrames) const {
  assert(numFrames > 0);
  std::vector<ImGuiRenderer::FrameResources> resources;
  resources.reserve(numFrames);
  std::generate_n(std::back_inserter(resources), numFrames,
                  std::bind_front(&ImGuiRenderer::createFrameResources, this));
  return resources;
}
ImGuiRenderer::FrameResources ImGuiRenderer::createFrameResources() const {
  auto &rd = getRenderDevice();
  return ImGuiRenderer::FrameResources{
    rd.createVertexBuffer(sizeof(ImDrawVert), UINT16_MAX,
                          rhi::AllocationHints::SequentialWrite),
    rd.createIndexBuffer(rhi::IndexType::UInt16, UINT16_MAX,
                         rhi::AllocationHints::SequentialWrite),
  };
}

void ImGuiRenderer::draw(rhi::CommandBuffer &cb, rhi::PixelFormat colorFormat,
                         FrameResources &resources,
                         const ImDrawData *drawData) {
  assert(drawData != nullptr);
  NAMED_DEBUG_MARKER(cb, "ImGui");
  TracyVkZone(cb.getTracyContext(), cb.getHandle(), "ImGui");

  // Scale coordinates for retina displays.
  // (screen coordinates != framebuffer coordinates).
  const auto framebufferExtent = rhi::Extent2D{
    .width = uint32_t(drawData->DisplaySize.x * drawData->FramebufferScale.x),
    .height = uint32_t(drawData->DisplaySize.y * drawData->FramebufferScale.y),
  };

  // Avoid rendering when minimized.
  if (!framebufferExtent) return;

  cb.setViewport({.extent = framebufferExtent});
  if (drawData->TotalVtxCount > 0) uploadGeometry(drawData, resources);

#if USE_PROJECTION_MATRIX
  const auto projectionMatrix =
    buildProjectionMatrix(drawData->DisplayPos, drawData->DisplaySize);
#else
  struct Uniforms {
    Uniforms(ImVec2 pos, ImVec2 size)
        : scale{2.0f / size.x, 2.0f / size.y},
          translate{-1.0f - pos.x * scale.x, -1.0f - pos.y * scale.y} {}

    glm::vec2 scale;
    glm::vec2 translate;
  };
  const Uniforms uniforms{drawData->DisplayPos, drawData->DisplaySize};
#endif

  auto descriptorSetBuilder = cb.createDescriptorSetBuilder();

  // Will project scissor/clipping rectangles into framebuffer space.
  // [0, 0] unless using multi-viewports.
  const glm::vec2 clipOffset{drawData->DisplayPos};
  // [1, 1] unless using retina display which are often [2, 2].
  const glm::vec2 clipScale{drawData->FramebufferScale};

  uint32_t globalVertexOffset{0};
  uint32_t globalIndexOffset{0};
  for (const auto *cmdList : drawData->CmdLists) {
    for (const auto &drawCmd : cmdList->CmdBuffer) {
      if (drawCmd.UserCallback != nullptr) {
        // User callback, registered via ImDrawList::AddCallback().
        // (ImDrawCallback_ResetRenderState is a special callback value used
        // by the user to request the renderer to reset render state).
        if (drawCmd.UserCallback == ImDrawCallback_ResetRenderState) {
          cb.setViewport({.extent = framebufferExtent});
        } else {
          drawCmd.UserCallback(cmdList, &drawCmd);
        }
      } else {
        const auto texture =
          static_cast<const rhi::Texture *>(drawCmd.TextureId);
        if (!texture || !*texture) continue;

        // Project scissor/clipping rectangles into framebuffer space.
        glm::vec2 clipMin{
          (drawCmd.ClipRect.x - clipOffset.x) * clipScale.x,
          (drawCmd.ClipRect.y - clipOffset.y) * clipScale.y,
        };
        glm::vec2 clipMax{
          (drawCmd.ClipRect.z - clipOffset.x) * clipScale.x,
          (drawCmd.ClipRect.w - clipOffset.y) * clipScale.y,
        };
        // Clamp to viewport, vkCmdSetScissor() won't accept values off bounds.
        clipMin = glm::max(clipMin, {0, 0});
        clipMax = glm::min(clipMax,
                           {framebufferExtent.width, framebufferExtent.height});

        if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) continue;

        cb.setScissor({
          .offset =
            {
              .x = int32_t(clipMin.x),
              .y = int32_t(clipMin.y),
            },
          .extent =
            {
              .width = uint32_t(clipMax.x - clipMin.x),
              .height = uint32_t(clipMax.y - clipMin.y),
            },
        });

        const auto *pipeline = _getPipeline(
          colorFormat, texture->getType() == rhi::TextureType::TextureCube);
        if (!pipeline) continue;

        constexpr auto kDescriptorSetId = 0;

        auto descriptorSet =
          descriptorSetBuilder
            .bind(0, rhi::bindings::CombinedImageSampler{texture})
            .build(pipeline->getDescriptorSetLayout(kDescriptorSetId));

        cb.bindPipeline(*pipeline)
#if USE_PROJECTION_MATRIX
          .pushConstants(rhi::ShaderStages::Vertex, 0, &projectionMatrix)
#else
          .pushConstants(rhi::ShaderStages::Vertex, 0, &uniforms)
#endif
          .bindDescriptorSet(kDescriptorSetId, descriptorSet)
          .draw({
            .vertexBuffer = &resources.vertexBuffer,
            .vertexOffset = drawCmd.VtxOffset + globalVertexOffset,
            .indexBuffer = &resources.indexBuffer,
            .indexOffset = drawCmd.IdxOffset + globalIndexOffset,
            .numIndices = drawCmd.ElemCount,
          });
      }
    }
    globalVertexOffset += cmdList->VtxBuffer.Size;
    globalIndexOffset += cmdList->IdxBuffer.Size;
  }
}

//
// (private):
//

rhi::GraphicsPipeline
ImGuiRenderer::_createPipeline(rhi::PixelFormat colorFormat,
                               bool cubemap) const {
  ShaderCodeBuilder shaderCodeBuilder{};
  const auto vertCode =
    shaderCodeBuilder.addDefine("USE_PROJECTION_MATRIX", USE_PROJECTION_MATRIX)
      .buildFromFile("UI.vert");

  shaderCodeBuilder.clearDefines();
  if (cubemap) {
    shaderCodeBuilder
      .addDefine("CUBEMAP", 1)
      // 0 = Disabled, 1 = Equirectangular, 2 = Cube net
      .addDefine("MODE", 2);
  } else {
    shaderCodeBuilder.addDefine("USE_MONOCHROMATIC_FONT",
                                USE_MONOCHROMATIC_FONT);
  }
  const auto fragCode = shaderCodeBuilder.buildFromFile("UI.frag");

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
          .type = rhi::VertexAttribute::Type::Float2,
          .offset = offsetof(ImDrawVert, uv),
        }},
      {2,
        {
          .type = rhi::VertexAttribute::Type::UByte4_Norm,
          .offset = offsetof(ImDrawVert, col),
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
    .build(getRenderDevice());
  // clang-format on
}
