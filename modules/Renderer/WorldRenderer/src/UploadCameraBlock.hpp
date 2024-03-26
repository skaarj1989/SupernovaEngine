#pragma once

#include "fg/Fwd.hpp"
#include "rhi/Extent2D.hpp"

namespace gfx {

class PerspectiveCamera;
struct RawCamera;

[[nodiscard]] FrameGraphResource
uploadCameraBlock(FrameGraph &, const rhi::Extent2D, const PerspectiveCamera &);
[[nodiscard]] FrameGraphResource
uploadCameraBlock(FrameGraph &, const rhi::Extent2D, const RawCamera &);

} // namespace gfx
