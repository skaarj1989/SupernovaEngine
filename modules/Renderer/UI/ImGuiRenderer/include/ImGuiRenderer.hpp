#pragma once

#include "rhi/RenderPass.hpp"
#include "rhi/FrameIndex.hpp"
#include "rhi/VertexBuffer.hpp"
#include "rhi/IndexBuffer.hpp"
#include "rhi/Texture.hpp"

namespace rhi {
class CommandBuffer;
}

struct ImDrawData;

class ImGuiRenderer final : public rhi::RenderPass<ImGuiRenderer> {
  friend class BasePass;

public:
  explicit ImGuiRenderer(rhi::RenderDevice &);
  ImGuiRenderer(const ImGuiRenderer &) = delete;
  ImGuiRenderer(ImGuiRenderer &&) noexcept = delete;
  ~ImGuiRenderer() = default;

  ImGuiRenderer &operator=(const ImGuiRenderer &) = delete;
  ImGuiRenderer &operator=(ImGuiRenderer &&) noexcept = delete;

  struct FrameResources {
    rhi::VertexBuffer vertexBuffer;
    rhi::IndexBuffer indexBuffer;
  };
  [[nodiscard]] std::vector<FrameResources>
  createResources(const rhi::FrameIndex::ValueType numFrames) const;
  [[nodiscard]] FrameResources createFrameResources() const;

  void draw(rhi::CommandBuffer &, const rhi::PixelFormat colorFormat,
            FrameResources &, const ImDrawData *);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const rhi::PixelFormat colorFormat, const bool cubemap) const;

private:
  rhi::Texture m_font;
};
