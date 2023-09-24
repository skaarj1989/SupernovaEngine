#pragma once

#include "rhi/RenderPass.hpp"
#include "RmlUi/Core/Vertex.h"

#include "glm/detail/type_mat4x4.hpp"
#include "glm/detail/type_vec2.hpp"

class RmlUiRenderer : public rhi::RenderPass<RmlUiRenderer> {
  friend class BasePass;

public:
  RmlUiRenderer() = delete;
  explicit RmlUiRenderer(rhi::RenderDevice &);
  RmlUiRenderer(const RmlUiRenderer &) = delete;
  RmlUiRenderer(RmlUiRenderer &&) noexcept = delete;
  ~RmlUiRenderer() = default;

  RmlUiRenderer &operator=(const RmlUiRenderer &) = delete;
  RmlUiRenderer &operator=(RmlUiRenderer &&) noexcept = delete;

  struct FrameResources {
    rhi::VertexBuffer vertexBuffer;
    uint32_t vertexOffset{0};
    rhi::IndexBuffer indexBuffer;
    uint32_t indexOffset{0};
  };
  [[nodiscard]] std::vector<FrameResources>
  createResources(int32_t numFrames) const;
  [[nodiscard]] FrameResources createFrameResources() const;

  struct TriangleList {
    std::span<Rml::Vertex> vertices;
    std::span<int32_t> indices;
  };
  struct Uniforms {
    glm::mat4 transform{1.0f};
    glm::vec2 translate{0.0f};
  };
  void draw(rhi::CommandBuffer &, const rhi::PixelFormat, FrameResources &,
            const TriangleList &, const rhi::Texture *, const Uniforms &);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(rhi::PixelFormat colorFormat, bool textured) const;
};
