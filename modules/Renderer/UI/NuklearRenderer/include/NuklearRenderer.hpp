#pragma once

#include "rhi/RenderDevice.hpp"
#include "Nuklear/nuklear.h"
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
  createResources(int32_t numFrames) const;
  [[nodiscard]] FrameResources createFrameResources() const;

  void draw(rhi::CommandBuffer &, FrameResources &, rhi::Extent2D,
            glm::vec2 scale = {1.0f, 1.0f});

private:
  void _uploadGeometry(FrameResources &) const;
  void _setupRenderState(rhi::CommandBuffer &, rhi::Extent2D) const;

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
