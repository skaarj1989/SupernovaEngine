#pragma once

#include "os/InputEvents.hpp"
#include <functional>

namespace rhi {
class CommandBuffer;
}

class WidgetWindow {
public:
  virtual ~WidgetWindow() = default;

  virtual void show(const char *name, bool *open) = 0;

  virtual void onInput(const os::InputEvent &) {}
  virtual void onUpdate(const float dt) {}
  virtual void onPhysicsUpdate(const float dt) {}
  virtual void onRender(rhi::CommandBuffer &, const float dt) {}
};

class SimpleWidgetWindow final : public WidgetWindow {
public:
  using OnShow = std::function<void(const char *, bool *)>;

  explicit SimpleWidgetWindow(OnShow onShow) : m_onShow{std::move(onShow)} {}

  void show(const char *name, bool *open) override { m_onShow(name, open); }

private:
  OnShow m_onShow;
};
