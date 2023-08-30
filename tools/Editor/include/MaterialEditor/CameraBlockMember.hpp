#pragma once

#include "MaterialEditor/Nodes/NodeCommon.hpp"

enum class CameraBlockMember {
  Projection,
  InversedProjection,
  View,
  InversedView,
  ViewProjection,
  InversedViewProjection,
  Resolution,
  Near,
  Far
};

[[nodiscard]] DataType getDataType(CameraBlockMember);

[[nodiscard]] const char *toString(CameraBlockMember);

[[nodiscard]] NodeResult evaluate(MaterialGenerationContext &, int32_t id,
                                  CameraBlockMember);
