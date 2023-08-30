#pragma once

#include "renderer/MaterialProperty.hpp"
#include <vector>

bool inspect(std::vector<gfx::Property> &);
bool inspect(const char *name, gfx::Property::Value &);
