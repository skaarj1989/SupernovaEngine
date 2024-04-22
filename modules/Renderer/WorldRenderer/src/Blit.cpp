#include "renderer/Blit.hpp"
#include "rhi/CommandBuffer.hpp"

#include "renderer/CommonSamplers.hpp"
#include "renderer/PostProcess.hpp"

#include "fg/FrameGraph.hpp"
#include "FrameGraphResourceAccess.hpp"
#include "renderer/FrameGraphTexture.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

namespace std {

template <> struct hash<gfx::Blit::MixPassInfo> {
  auto operator()(const gfx::Blit::MixPassInfo &passInfo) const noexcept {
    size_t h{0};
    hashCombine(h, passInfo.colorFormat);
    return h;
  };
};
template <> struct hash<gfx::Blit::MergePassInfo> {
  auto operator()(const gfx::Blit::MergePassInfo &passInfo) const noexcept {
    size_t h{0};
    hashCombine(h, passInfo.colorFormat, passInfo.numTextures);
    return h;
  };
};

} // namespace std

namespace gfx {

Blit::Blit(rhi::RenderDevice &rd, const CommonSamplers &commonSamplers)
    : BasePass{rd}, m_samplers{commonSamplers} {}

uint32_t Blit::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? BasePass::count() : 0;
}
void Blit::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) BasePass::clear();
}

FrameGraphResource Blit::mix(FrameGraph &fg, const FrameGraphResource a,
                             const FrameGraphResource b) {
  static constexpr auto kPassName = "Mix";
  ZoneScopedN(kPassName);

  struct Data {
    FrameGraphResource output;
  };
  const auto [output] = fg.addCallbackPass<Data>(
    kPassName,
    [&fg, a, b](FrameGraph::Builder &builder, Data &data) {
      PASS_SETUP_ZONE;

      for (auto index = 1u; auto input : {a, b}) {
        builder.read(
          input, TextureRead{.binding =
                               {
                                 .location = {.set = 0, .binding = index},
                                 .pipelineStage = PipelineStage::FragmentShader,
                               },
                             .type = TextureRead::Type::SampledImage});
        ++index;
      }

      const auto &inputDescA = fg.getDescriptor<FrameGraphTexture>(a);
      const auto &inputDescB = fg.getDescriptor<FrameGraphTexture>(b);
      assert(inputDescA.format == inputDescB.format);
      data.output = builder.create<FrameGraphTexture>(
        "Mixed",
        {
          .extent = inputDescA.extent,
          .format = inputDescA.format,
          .usageFlags = (inputDescA.usageFlags | inputDescB.usageFlags) &
                        ~rhi::ImageUsage::Storage,
        });
      data.output = builder.write(data.output, Attachment{.index = 0});
    },
    [this](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      const auto *pipeline = _getPipeline(MixPassInfo{
        .colorFormat = rhi::getColorFormat(*framebufferInfo, 0),
      });
      if (pipeline) {
        sets[0][0] = rhi::bindings::SeparateSampler{m_samplers.bilinear};
        renderFullScreenPostProcess(rc, *pipeline);
      }
    });

  return output;
}

FrameGraphResource Blit::addColor(FrameGraph &fg,
                                  const FrameGraphResource target,
                                  const FrameGraphResource source) {
  return merge(fg, target, {source});
}

FrameGraphResource Blit::merge(FrameGraph &fg, FrameGraphResource target,
                               const std::vector<FrameGraphResource> &sources) {
  static constexpr auto kPassName = "Merge";
  ZoneScopedN(kPassName);

  const auto numTextures = sources.size();
  assert(numTextures > 0);

  fg.addCallbackPass(
    kPassName,
    [&target, &sources](FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      for (auto index = 1u; const auto input : sources) {
        builder.read(input,
                     TextureRead{
                       .binding =
                         {
                           .location = {.set = 0, .binding = index},
                           .pipelineStage = PipelineStage::FragmentShader,
                         },
                       .type = TextureRead::Type::SampledImage,
                     });
      }
      target = builder.write(target, Attachment{.index = 0});
    },
    [this, numTextures](const auto &, const FrameGraphPassResources &,
                        void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      const auto *pipeline = _getPipeline(MergePassInfo{
        .colorFormat = rhi::getColorFormat(*framebufferInfo, 0),
        .numTextures = numTextures,
      });
      if (pipeline) {
        sets[0][0] = rhi::bindings::SeparateSampler{m_samplers.bilinear};
        renderFullScreenPostProcess(rc, *pipeline);
      }
    });

  return target;
}

//
// (private):
//

rhi::GraphicsPipeline Blit::_createPipeline(const MixPassInfo &passInfo) const {
  return createPostProcessPipeline(
    getRenderDevice(), passInfo.colorFormat,
    ShaderCodeBuilder{}.buildFromFile("Mix.frag"));
}

rhi::GraphicsPipeline
Blit::_createPipeline(const MergePassInfo &passInfo) const {
  ShaderCodeBuilder shaderCodeBuilder;

  // clang-format off
  return rhi::GraphicsPipeline::Builder{}
    .setColorFormats({passInfo.colorFormat})
    .setInputAssembly({})
    .addShader(rhi::ShaderType::Vertex,
               shaderCodeBuilder.buildFromFile("FullScreenTriangle.vert"))
    .addShader(rhi::ShaderType::Fragment,
               shaderCodeBuilder
                 .addDefine("NUM_TEXTURES", passInfo.numTextures)
                 .buildFromFile("Merge.frag"))

    .setDepthStencil({
      .depthTest = false,
      .depthWrite = false,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Front,
    })
    .setBlending(0, {
      .enabled = true,
      .srcColor = rhi::BlendFactor::One,
      .dstColor = rhi::BlendFactor::One,
    })
    .build(getRenderDevice());
  // clang-format on
}

} // namespace gfx
