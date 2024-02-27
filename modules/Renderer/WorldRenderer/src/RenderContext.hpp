#pragma once

#include "rhi/RenderDevice.hpp"
#include "renderer/ForwardPassInfo.hpp"

namespace gfx {

using ResourceBindings =
  robin_hood::unordered_map<uint32_t, rhi::ResourceBinding>;
using ResourceSet = robin_hood::unordered_map<uint32_t, ResourceBindings>;

[[nodiscard]] std::string toString(const ResourceSet &);

struct RenderContext {
  explicit RenderContext(rhi::CommandBuffer &);

  rhi::CommandBuffer &commandBuffer;
  std::optional<rhi::FramebufferInfo> framebufferInfo;
  ResourceSet resourceSet;
};

void overrideSampler(rhi::ResourceBinding &, VkSampler);

struct Batch;

[[nodiscard]] BaseGeometryPassInfo adjust(BaseGeometryPassInfo, const Batch &);

void render(RenderContext &, const rhi::GraphicsPipeline &, const Batch &);
void bindBatch(RenderContext &, const Batch &);
void bindMaterialTextures(RenderContext &, const TextureResources &);
void bindDescriptorSets(RenderContext &, const rhi::BasePipeline &);
void drawBatch(RenderContext &, const Batch &);

void renderFullScreenPostProcess(RenderContext &,
                                 const rhi::GraphicsPipeline &);

void endRendering(RenderContext &);

} // namespace gfx

#define PASS_SETUP_ZONE ZoneScopedN("SetupPass")
