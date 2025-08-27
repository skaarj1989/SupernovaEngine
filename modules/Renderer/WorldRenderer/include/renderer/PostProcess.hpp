#pragma once

#include "rhi/GraphicsPipeline.hpp"
#include <filesystem>

namespace gfx {

[[nodiscard]] rhi::GraphicsPipeline
createPostProcessPipelineFromFile(rhi::RenderDevice &,
                                  const rhi::PixelFormat colorFormat,
                                  const rhi::SlangCompilationRequest &);

[[nodiscard]] rhi::GraphicsPipeline
createPostProcessPipelineFromFile(rhi::RenderDevice &,
                                  const rhi::PixelFormat colorFormat,
                                  const std::filesystem::path &);
[[nodiscard]] rhi::GraphicsPipeline
createPostProcessPipeline(rhi::RenderDevice &,
                          const rhi::PixelFormat colorFormat,
                          const std::string &fragCode);

} // namespace gfx
