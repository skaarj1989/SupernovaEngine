#pragma once

#include "DataType.hpp"

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

[[nodiscard]] DataType getDataType(const BuiltInConstant);
[[nodiscard]] const char *toString(const BuiltInConstant);

enum class BuiltInSampler {
  SceneDepth,
  SceneColor,

  COUNT,
};

[[nodiscard]] DataType getDataType(const BuiltInSampler);
[[nodiscard]] const char *toString(const BuiltInSampler);
