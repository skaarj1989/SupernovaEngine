#pragma once

#include "fg/Fwd.hpp"

namespace gfx {

struct FrameInfo {
  float time;
  float deltaTime;
};
void uploadFrameBlock(FrameGraph &, FrameGraphBlackboard &, const FrameInfo &);

} // namespace gfx
