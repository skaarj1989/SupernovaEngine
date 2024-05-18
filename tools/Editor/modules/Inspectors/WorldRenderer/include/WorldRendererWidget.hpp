#pragma once

#include "WidgetWindow.hpp"
#include "imgui.h"

namespace gfx {
class WorldRenderer;
}

class WorldRendererWidget final : public WidgetWindow {
public:
  explicit WorldRendererWidget(gfx::WorldRenderer &);

  void show(const char *name, bool *open) override;

private:
  gfx::WorldRenderer &m_renderer;
  float m_time{0.0f};

  struct ScrollingBuffer {
    const int32_t capacity{0};
    int32_t offset{0};
    ImVector<ImVec2> data;

    explicit ScrollingBuffer(const int32_t capacity_ = 2000);
    void addPoint(const float t, const float value);
  };
  ScrollingBuffer m_textures;
  ScrollingBuffer m_buffers;
};
