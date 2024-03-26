#pragma once

#include "fg/Fwd.hpp"
#include <string>

namespace rhi {
class Texture;
}

namespace gfx {

[[nodiscard]] FrameGraphResource
importTexture(FrameGraph &, const std::string_view name, rhi::Texture *);

} // namespace gfx
