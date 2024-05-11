#pragma once

#include "DataType.hpp"

enum class FrameBlockMember { Time, DeltaTime, COUNT };

[[nodiscard]] DataType getDataType(const FrameBlockMember);
[[nodiscard]] const char *toString(const FrameBlockMember);
