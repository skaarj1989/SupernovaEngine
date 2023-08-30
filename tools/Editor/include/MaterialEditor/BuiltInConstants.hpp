#pragma once

#include "MaterialEditor/Nodes/NodeCommon.hpp"

enum class BuiltInConstant {
  // Camera:

  CameraPosition,
  ScreenTexelSize,
  AspectRatio,

  // ---

  ModelMatrix,
  FragPosWorldSpace,
  FragPosViewSpace,

  // ---

  ViewDir,

  COUNT,
};

[[nodiscard]] DataType getDataType(BuiltInConstant);

[[nodiscard]] const char *toString(BuiltInConstant);

[[nodiscard]] NodeResult evaluate(MaterialGenerationContext &, int32_t id,
                                  BuiltInConstant);

enum class BuiltInSampler {
  SceneDepth,
  SceneColor,

  COUNT,
};

[[nodiscard]] DataType getDataType(BuiltInSampler);

[[nodiscard]] const char *toString(BuiltInSampler);

[[nodiscard]] NodeResult evaluate(MaterialGenerationContext &, int32_t id,
                                  BuiltInSampler);
