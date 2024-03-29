#pragma once

#include "rhi/FrameIndex.hpp"
#include "rhi/VertexBuffer.hpp"
#include "rhi/IndexBuffer.hpp"
#include "rhi/Texture.hpp"
#include "rhi/GraphicsPipeline.hpp"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include "Nuklear/nuklear.h"
#ifdef _MSC_VER
#  pragma warning(pop)
#endif
#include "glm/ext/vector_float2.hpp"

// https://immediate-mode-ui.github.io/Nuklear/doc/nuklear.html

class NuklearRenderer final {
public:
  NuklearRenderer() = delete;
  NuklearRenderer(rhi::RenderDevice &, nk_context &, nk_font_atlas &);
  NuklearRenderer(const NuklearRenderer &) = delete;
  NuklearRenderer(NuklearRenderer &&) noexcept = delete;
  ~NuklearRenderer();

  NuklearRenderer &operator=(const NuklearRenderer &) = delete;
  NuklearRenderer &operator=(NuklearRenderer &&) noexcept = delete;

  struct FrameResources {
    rhi::VertexBuffer vertexBuffer;
    rhi::IndexBuffer indexBuffer;
  };
  [[nodiscard]] std::vector<FrameResources>
  createResources(const rhi::FrameIndex::ValueType numFrames) const;
  [[nodiscard]] FrameResources createFrameResources() const;

  void draw(rhi::CommandBuffer &, FrameResources &, const rhi::Extent2D,
            const glm::vec2 scale = {1.0f, 1.0f});

private:
  void _uploadGeometry(FrameResources &) const;
  void _setupRenderState(rhi::CommandBuffer &, const rhi::Extent2D) const;

private:
  nk_context &m_ctx;
  nk_convert_config m_config{};

  rhi::RenderDevice &m_renderDevice;

  rhi::Texture m_font;
  rhi::GraphicsPipeline m_graphicsPipeline;

  nk_buffer m_commands;
  nk_buffer m_vertices;
  nk_buffer m_indices;
};
