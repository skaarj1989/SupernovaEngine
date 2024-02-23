#pragma once

#include "RmlUiRenderer.hpp"
#include "RmlUi/Core/RenderInterface.h"

struct RmlUiRenderData {
  RmlUiRenderData() = default;
  RmlUiRenderData(const RmlUiRenderer &,
                  const rhi::FrameIndex::ValueType numFramesInFlight);
  RmlUiRenderData(const RmlUiRenderData &);
  RmlUiRenderData(RmlUiRenderData &&) noexcept = default;
  ~RmlUiRenderData() = default;

  RmlUiRenderData &operator=(const RmlUiRenderData &) noexcept = delete;
  RmlUiRenderData &operator=(RmlUiRenderData &&) noexcept = default;

  void resize(const rhi::Extent2D &);
  [[nodiscard]] RmlUiRenderer::FrameResources *swapBuffers();

  void transform(std::optional<glm::mat4>);
  void translate(const glm::vec2);

  std::vector<RmlUiRenderer::FrameResources> frameResources;
  rhi::FrameIndex frameIndex;

  glm::mat4 projection{1.0f};
  RmlUiRenderer::Uniforms uniforms;

  rhi::Rect2D scissorOriginal;
  rhi::Rect2D scissor;

  bool isTransformEnabled{false};
  bool isUseScissorSpecified{false};
};

class RmlUiRenderInterface : public Rml::RenderInterface {
public:
  explicit RmlUiRenderInterface(rhi::RenderDevice &);

  void Set(rhi::CommandBuffer &, rhi::Texture &target, RmlUiRenderData &);

  void RenderGeometry(Rml::Vertex *, int32_t numVertices, int32_t *indices,
                      int32_t numIndices, Rml::TextureHandle,
                      const Rml::Vector2f &translation) override;

  void EnableScissorRegion(bool) override;
  void SetScissorRegion(int x, int y, int width, int height) override;

  bool LoadTexture(Rml::TextureHandle &, Rml::Vector2i &dimensions,
                   const Rml::String &source) override;
  bool GenerateTexture(Rml::TextureHandle &, const Rml::byte *,
                       const Rml::Vector2i &dimensions) override;
  void ReleaseTexture(Rml::TextureHandle) override;

  void SetTransform(const Rml::Matrix4f *) override;

  RmlUiRenderer &GetRenderer();
  [[nodiscard]] RmlUiRenderData
  CreateRenderData(const rhi::FrameIndex::ValueType numFrames) const;

private:
  std::unique_ptr<RmlUiRenderer> m_renderer;

  rhi::CommandBuffer *m_commandBuffer{nullptr};
  const rhi::Texture *m_target{nullptr};
  RmlUiRenderData *m_renderData{nullptr};
  RmlUiRenderer::FrameResources *m_frameResources{nullptr};
};
