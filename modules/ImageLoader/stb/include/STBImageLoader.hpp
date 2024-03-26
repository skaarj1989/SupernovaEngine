#pragma once

#include "rhi/Texture.hpp"
#include <filesystem>
#include <expected>

namespace rhi {
class RenderDevice;
}

[[nodiscard]] std::expected<rhi::Texture, std::string>
loadTextureSTB(const std::filesystem::path &, rhi::RenderDevice &);
