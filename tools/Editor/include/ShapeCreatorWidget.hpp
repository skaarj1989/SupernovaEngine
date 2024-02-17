#pragma once

#include "WidgetWindow.hpp"
#include "physics/ShapeBuilder.hpp"

class ShapeCreatorWidget final : public WidgetWindow {
public:
  void show(const char *name, bool *open) override;

private:
  ShapeBuilder m_builder;
};
