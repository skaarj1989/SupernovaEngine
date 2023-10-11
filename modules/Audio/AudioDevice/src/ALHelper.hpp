#pragma once

#include "AL/al.h"
#include "ClipInfo.hpp"

namespace audio {

[[nodiscard]] ALenum pickFormat(const ClipInfo &);

} // namespace audio
