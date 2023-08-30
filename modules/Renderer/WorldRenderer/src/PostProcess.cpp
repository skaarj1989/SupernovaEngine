#include "renderer/PostProcess.hpp"
#include "rhi/RenderDevice.hpp"
#include "ShaderCodeBuilder.hpp"

namespace gfx {

rhi::GraphicsPipeline
createPostProcessPipelineFromFile(rhi::RenderDevice &rd,
                                  rhi::PixelFormat colorFormat,
                                  const std::filesystem::path &p) {
  return createPostProcessPipeline(rd, colorFormat,
                                   ShaderCodeBuilder{}.buildFromFile(p));
}
rhi::GraphicsPipeline
createPostProcessPipeline(rhi::RenderDevice &rd, rhi::PixelFormat colorFormat,
                          const std::string_view fragCode) {
  return rhi::GraphicsPipeline::Builder{}
    .setColorFormats({colorFormat})
    .setInputAssembly({})
    .addShader(rhi::ShaderType::Vertex,
               ShaderCodeBuilder{}.buildFromFile("FullScreenTriangle.vert"))
    .addShader(rhi::ShaderType::Fragment, fragCode)

    .setDepthStencil({
      .depthTest = false,
      .depthWrite = false,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Front,
    })
    .setBlending(0, {.enabled = false})
    .build(rd);
}

} // namespace gfx
