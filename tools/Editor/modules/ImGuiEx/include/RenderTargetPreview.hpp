#pragma once

#include "rhi/Texture.hpp"
#include "imgui.h"
#include "glm/ext/vector_int2.hpp"
#include "glm/ext/vector_uint2.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/vector_relational.hpp" // any, notEqual

namespace rhi {
void prepareForReading(CommandBuffer &, const Texture &);
}

class RenderTargetPreview {
public:
  explicit RenderTargetPreview(rhi::RenderDevice &);

  template <typename OnResizeCallback = std::monostate,
            typename OnShowCallback = std::monostate>
  void show(OnResizeCallback onResize, OnShowCallback onShow) {
    const glm::uvec2 contentSize = glm::vec2{ImGui::GetContentRegionAvail()};
    if (ImGui::BeginChild("##PREVIEW")) {
      _present(contentSize);
      if constexpr (std::is_invocable_v<OnShowCallback>) {
        onShow();
      }
      m_requiresPaint = true;
    }
    // Resize (if necessary) after display (to avoid blink).
    if (_isAreaValid(contentSize)) {
      const auto lastSize = glm::uvec2{m_target.getExtent()};
      if (glm::any(glm::notEqual(contentSize, lastSize))) {
        _resize(contentSize);
        if constexpr (std::is_invocable_v<OnResizeCallback, rhi::Extent2D>) {
          onResize(m_target.getExtent());
        }
      }
    }
    ImGui::EndChild();
  }

  template <typename Func> void render(rhi::CommandBuffer &cb, Func onRender) {
    if (m_requiresPaint && m_target) {
      onRender(m_target);
      m_requiresPaint = false;
      rhi::prepareForReading(cb, m_target);
    }
  }

  rhi::RenderDevice &getRenderDevice() { return m_renderDevice; }

  [[nodiscard]] glm::ivec2 getPosition() const;
  [[nodiscard]] rhi::Extent2D getExtent() const;

private:
  [[nodiscard]] bool _isAreaValid(const glm::uvec2) const;
  void _resize(const glm::uvec2);

  void _present(const glm::vec2 size);

protected:
  rhi::RenderDevice &m_renderDevice;

private:
  rhi::Texture m_target;
  bool m_requiresPaint{false};
  glm::ivec2 m_position{};
};
