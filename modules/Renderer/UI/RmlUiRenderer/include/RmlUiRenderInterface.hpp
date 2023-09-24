#pragma once

#include "RmlUiRenderer.hpp"
#include "RmlUi/Core/RenderInterface.h"
#include "rhi/RenderTargetView.hpp"

class RmlUiRenderInterface : public Rml::RenderInterface {
public:
  RmlUiRenderInterface(RmlUiRenderer &, const int32_t numFrames);

  void Set(rhi::CommandBuffer &, const rhi::RenderTargetView);

  void RenderGeometry(Rml::Vertex *, int32_t numVertices, int32_t *indices,
                      int32_t numIndices, Rml::TextureHandle,
                      const Rml::Vector2f &translation) override;

  void EnableScissorRegion(bool) override;
  void SetScissorRegion(int x, int y, int width, int height) override;

  bool LoadTexture(Rml::TextureHandle &, Rml::Vector2i &dimensions,
                   const Rml::String &source) override;
  bool GenerateTexture(Rml::TextureHandle &, const byte *,
                       const Rml::Vector2i &dimensions) override;
  void ReleaseTexture(Rml::TextureHandle) override;

  void SetTransform(const Rml::Matrix4f *) override;

private:
  RmlUiRenderer &m_renderer;

  rhi::CommandBuffer *m_commandBuffer{nullptr};
  const rhi::Texture *m_target{nullptr};

  std::vector<RmlUiRenderer::FrameResources> m_frameResources;
  int32_t m_frameIndex{0};

  glm::mat4 m_projection{1.0f};
  RmlUiRenderer::Uniforms m_uniforms;

  rhi::Rect2D m_scissorOriginal;
  rhi::Rect2D m_scissor;

  bool m_isTransformEnabled{false};
  bool m_isUseScissorSpecified{false};
};
