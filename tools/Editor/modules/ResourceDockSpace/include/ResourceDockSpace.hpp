#pragma once

#include "WidgetCache.hpp"

class ResourcesWidget final : public WidgetWindow {
public:
  ResourcesWidget();
  void show(const char *name, bool *open) override;

private:
  void _setupDockSpace(const ImGuiID) const;

private:
  struct WidgetConfig {
    const char *name{nullptr};
    bool open{false};
  };
  WidgetCache<WidgetConfig> m_widgets;
};
