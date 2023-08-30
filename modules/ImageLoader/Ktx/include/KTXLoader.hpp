#pragma once

#include "rhi/RenderDevice.hpp"
#include <filesystem>

[[nodiscard]] std::expected<rhi::Texture, std::string>
loadTextureKTX(const std::filesystem::path &, rhi::RenderDevice &);
