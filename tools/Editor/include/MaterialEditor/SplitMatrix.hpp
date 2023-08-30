#pragma once

#include "MaterialEditor/Nodes/NodeCommon.hpp"

enum class SplitMatrix {
  Column0,
  Column1,
  Column2,
  Column3,
};

[[nodiscard]] NodeResult evaluate(MaterialGenerationContext &, int32_t id,
                                  SplitMatrix);
