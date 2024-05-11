#pragma once

#include "DataType.hpp"

enum class Attribute {
  Position,
  TexCoord0,
  TexCoord1,
  Normal,
  Color,

  COUNT,
};

[[nodiscard]] DataType getDataType(const Attribute);
[[nodiscard]] const char *toString(const Attribute);
