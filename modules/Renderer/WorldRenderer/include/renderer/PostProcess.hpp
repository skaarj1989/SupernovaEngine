#pragma once

#include "rhi/GraphicsPipeline.hpp"
#include <filesystem>

namespace gfx {

[[nodiscard]] rhi::GraphicsPipeline
createPostProcessPipelineFromFile(rhi::RenderDevice &,
                                  const rhi::PixelFormat colorFormat,
                                  const std::filesystem::path &);
[[nodiscard]] rhi::GraphicsPipeline
createPostProcessPipeline(rhi::RenderDevice &,
                          const rhi::PixelFormat colorFormat,
                          const std::string_view fragCode);

} // namespace gfx
