#pragma once

#include "MaterialEditor/Nodes/NodeCommon.hpp"

enum class Attribute {
  Position,
  TexCoord0,
  TexCoord1,
  Normal,
  Color,

  COUNT,
};

[[nodiscard]] DataType getDataType(Attribute);

[[nodiscard]] const char *toString(Attribute);

[[nodiscard]] NodeResult evaluate(MaterialGenerationContext &, int32_t id,
                                  Attribute);
