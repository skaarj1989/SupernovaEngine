#pragma once

#include "rhi/DescriptorSetBuilder.hpp"
#include "rhi/FramebufferInfo.hpp"
#include "renderer/CommonSamplers.hpp"
#include "renderer/Material.hpp"
#include "PipelineStage.hpp"
#include "BaseGeometryPassInfo.hpp"

namespace rhi {
class BasePipeline;
class GraphicsPipeline;
class CommandBuffer;
} // namespace rhi

namespace gfx {

using ResourceBindings =
  robin_hood::unordered_map<rhi::BindingIndex, rhi::ResourceBinding>;
using ResourceSet =
  robin_hood::unordered_map<rhi::DescriptorSetIndex, ResourceBindings>;

[[nodiscard]] std::string toString(const ResourceSet &);

[[nodiscard]] rhi::PipelineStages convert(const PipelineStage);

struct RenderContext {
  RenderContext(rhi::CommandBuffer &, const CommonSamplers &);

  rhi::CommandBuffer &commandBuffer;
  const CommonSamplers &commonSamplers;

  std::optional<rhi::FramebufferInfo> framebufferInfo;
  ResourceSet resourceSet;
};

void overrideSampler(rhi::ResourceBinding &, const VkSampler);

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
