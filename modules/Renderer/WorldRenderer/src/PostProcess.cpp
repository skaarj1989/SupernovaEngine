#include "renderer/PostProcess.hpp"
#include "rhi/RenderDevice.hpp"
#include "ShaderCodeBuilder.hpp"

namespace gfx {

namespace {

template <typename Func>
rhi::GraphicsPipeline
createPostProcessPipeline(rhi::RenderDevice &rd,
                          const rhi::PixelFormat colorFormat, Func f) {
  rhi::GraphicsPipeline::Builder builder;
  builder.setColorFormats({colorFormat})
    .setInputAssembly({})
    .setDepthStencil({
      .depthTest = false,
      .depthWrite = false,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Front,
    })
    .setBlending(0, {.enabled = false});
  f(builder);
  return builder.build(rd);
}

} // namespace

rhi::GraphicsPipeline
createPostProcessPipelineFromFile(rhi::RenderDevice &rd,
                                  const rhi::PixelFormat colorFormat,
                                  const rhi::SlangCompilationRequest &request) {
  return createPostProcessPipeline(
    rd, colorFormat, [&request](rhi::GraphicsPipeline::Builder &builder) {
      builder.loadProgram(request);
    });
}

rhi::GraphicsPipeline
createPostProcessPipelineFromFile(rhi::RenderDevice &rd,
                                  const rhi::PixelFormat colorFormat,
                                  const std::filesystem::path &p) {
  return createPostProcessPipeline(rd, colorFormat,
                                   ShaderCodeBuilder{}.buildFromFile(p));
}
rhi::GraphicsPipeline
createPostProcessPipeline(rhi::RenderDevice &rd,
                          const rhi::PixelFormat colorFormat,
                          const std::string &fragCode) {
  return createPostProcessPipeline(
    rd, colorFormat, [fragCode](rhi::GraphicsPipeline::Builder &builder) {
      builder
        .addShader(rhi::ShaderType::Vertex,
                   ShaderCodeBuilder{}.buildFromFile("FullScreenTriangle.vert"))
        .addShader(rhi::ShaderType::Fragment, fragCode);
    });
}

} // namespace gfx
