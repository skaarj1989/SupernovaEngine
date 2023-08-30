#pragma once

#include "MaterialEditor/Nodes/NodeCommon.hpp"

enum class FrameBlockMember { Time, DeltaTime };

[[nodiscard]] DataType getDataType(FrameBlockMember);

[[nodiscard]] const char *toString(FrameBlockMember);

[[nodiscard]] NodeResult evaluate(MaterialGenerationContext &, int32_t id,
                                  FrameBlockMember);
