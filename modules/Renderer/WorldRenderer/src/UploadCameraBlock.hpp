#pragma once

#include "fg/Fwd.hpp"
#include "rhi/Extent2D.hpp"
#include "PerspectiveCamera.hpp"
#include "renderer/RawCamera.hpp"

namespace gfx {

[[nodiscard]] FrameGraphResource uploadCameraBlock(FrameGraph &, rhi::Extent2D,
                                                   const PerspectiveCamera &);
[[nodiscard]] FrameGraphResource uploadCameraBlock(FrameGraph &, rhi::Extent2D,
                                                   const RawCamera &);

} // namespace gfx
