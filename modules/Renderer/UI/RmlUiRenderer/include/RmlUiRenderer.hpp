#pragma once

#include "rhi/RenderPass.hpp"
#include "rhi/FrameIndex.hpp"
#include "rhi/VertexBuffer.hpp"
#include "rhi/IndexBuffer.hpp"

#include "RmlUi/Core/Vertex.h"

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/matrix_float4x4.hpp"

namespace rhi {
class Texture;
class CommandBuffer;
} // namespace rhi

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
  createResources(const rhi::FrameIndex::ValueType numFrames) const;
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
  _createPipeline(const rhi::PixelFormat colorFormat,
                  const bool textured) const;
};
