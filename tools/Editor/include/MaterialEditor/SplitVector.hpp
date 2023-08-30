#pragma once

#include "MaterialEditor/Nodes/NodeCommon.hpp"

enum class SplitVector {
  X,
  Y,
  Z,
  W,
  XYZ,

  R = X,
  G = Y,
  B = Z,
  A = W,
  RGB = XYZ,
};

[[nodiscard]] const char *getPostfix(SplitVector);

[[nodiscard]] NodeResult evaluate(MaterialGenerationContext &, int32_t id,
                                  SplitVector);
