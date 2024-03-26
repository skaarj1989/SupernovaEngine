#pragma once

#include "fg/FrameGraph.hpp"

class FrameGraphBlackboard;

namespace gfx {

void read(FrameGraph::Builder &, const FrameGraphBlackboard &,
          const FrameGraphResource instances);

} // namespace gfx
