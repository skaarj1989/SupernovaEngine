#pragma once

#include "DataType.hpp"
#include "renderer/MaterialProperty.hpp"

using PropertyVariant = gfx::Property::Value;

[[nodiscard]] DataType getDataType(const PropertyVariant &);
[[nodiscard]] const char *toString(const PropertyVariant &);
