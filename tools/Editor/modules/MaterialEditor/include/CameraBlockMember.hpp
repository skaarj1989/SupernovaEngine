#pragma once

#include "DataType.hpp"

enum class CameraBlockMember {
  Projection,
  InversedProjection,
  View,
  InversedView,
  ViewProjection,
  InversedViewProjection,
  Resolution,
  Near,
  Far,

  COUNT,
};

[[nodiscard]] DataType getDataType(const CameraBlockMember);
[[nodiscard]] const char *toString(const CameraBlockMember);
