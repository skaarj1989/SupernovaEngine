#pragma once

#include "AL/al.h"

namespace audio {

struct ClipInfo;

[[nodiscard]] ALenum pickFormat(const ClipInfo &);

} // namespace audio
