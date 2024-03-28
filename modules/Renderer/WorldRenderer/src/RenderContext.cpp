#include "RenderContext.hpp"
#include "rhi/CommandBuffer.hpp"
#include "rhi/GraphicsPipeline.hpp"
#include "Batch.hpp"
#include "renderer/MeshInstance.hpp"

#include <ranges>
#include <format>

namespace gfx {

namespace {

void bindTextures(ResourceBindings &bindings, const uint32_t firstBinding,
                  const TextureResources &textures) {
  bindings.clear();

  for (auto [i, textureInfo] :
       textures | std::views::values | std::views::enumerate) {
    bindings[firstBinding + i] = rhi::bindings::CombinedImageSampler{
      .texture = textureInfo.texture.get(),
    };
  }
}
void changeOffset(rhi::ResourceBinding &v, const uint32_t offset) {
  std::get<rhi::bindings::StorageBuffer>(v).offset = offset;
}

void bindDescriptorSets(rhi::CommandBuffer &cb,
                        const rhi::BasePipeline &pipeline,
                        const ResourceSet &sets) {
  auto descriptorSetBuilder = cb.createDescriptorSetBuilder();
  for (const auto &[set, bindings] : sets) {
    for (const auto &[index, info] : bindings) {
      descriptorSetBuilder.bind(index, info);
    }
    const auto descriptors =
      descriptorSetBuilder.build(pipeline.getDescriptorSetLayout(set));
    cb.bindDescriptorSet(set, descriptors);
  }
}

[[nodiscard]] auto validate(const gfx::TextureResources &textures) {
  return std::ranges::all_of(textures,
                             [](const auto &p) { return p.second.isValid(); });
}

} // namespace

std::string toString(const ResourceSet &sets) {
  std::ostringstream oss;
  for (const auto &[set, bindings] : sets) {
    for (const auto &[index, info] : bindings) {
      std::ostream_iterator<std::string>{oss, "\n"} = std::format(
        "[set={}, binding={}] = {}", set, index, rhi::toString(info));
    }
  }
  return oss.str();
}

rhi::PipelineStages convert(const PipelineStage pipelineStage) {
  rhi::PipelineStages stageMask{rhi::PipelineStages::None};
  if (bool(pipelineStage & PipelineStage::VertexShader)) {
    stageMask |= rhi::PipelineStages::VertexShader;
  }
  if (bool(pipelineStage & PipelineStage::GeometryShader)) {
    stageMask |= rhi::PipelineStages::GeometryShader;
  }
  if (bool(pipelineStage & PipelineStage::FragmentShader)) {
    stageMask |= rhi::PipelineStages::FragmentShader;
  }
  if (bool(pipelineStage & PipelineStage::ComputeShader)) {
    stageMask |= rhi::PipelineStages::ComputeShader;
  }
  return stageMask;
}

void overrideSampler(rhi::ResourceBinding &v, VkSampler sampler) {
  assert(sampler != VK_NULL_HANDLE);
  std::get<rhi::bindings::CombinedImageSampler>(v).sampler = sampler;
}

BaseGeometryPassInfo adjust(BaseGeometryPassInfo info, const Batch &batch) {
  info.topology = batch.subMesh->topology;
  info.vertexFormat = std::addressof(batch.mesh->getVertexFormat());
  info.material = batch.material;
  return info;
}

void render(RenderContext &rc, const rhi::GraphicsPipeline &pipeline,
            const Batch &batch) {
  if (validate(batch.textures)) {
    bindBatch(rc, batch);
    rc.commandBuffer.bindPipeline(pipeline);
    bindDescriptorSets(rc, pipeline);
    drawBatch(rc, batch);
  }
}
void bindBatch(RenderContext &rc, const Batch &batch) {
  bindMaterialTextures(rc, batch.textures);
  if (hasProperties(*batch.material)) {
    changeOffset(rc.resourceSet[0][1], batch.materialOffset);
  }
}
void bindMaterialTextures(RenderContext &rc, const TextureResources &textures) {
  bindTextures(rc.resourceSet[3], 0, textures);
}
void bindDescriptorSets(RenderContext &rc, const rhi::BasePipeline &pipeline) {
  bindDescriptorSets(rc.commandBuffer, pipeline, rc.resourceSet);
}
void drawBatch(RenderContext &rc, const Batch &batch) {
  rc.commandBuffer
    .pushConstants(rhi::ShaderStages::Vertex, 0, &batch.instances.offset)
    .draw(getGeometryInfo(*batch.mesh, *batch.subMesh), batch.instances.count);
}

void renderFullScreenPostProcess(RenderContext &rc,
                                 const rhi::GraphicsPipeline &pipeline) {
  auto &cb = rc.commandBuffer;
  RHI_GPU_ZONE(cb, "FullScreenPostProcess");
  cb.bindPipeline(pipeline);
  bindDescriptorSets(rc, pipeline);
  cb.beginRendering(*rc.framebufferInfo).drawFullScreenTriangle();
  endRendering(rc);
}

void endRendering(RenderContext &rc) {
  rc.commandBuffer.endRendering();
  rc.framebufferInfo.reset();
  rc.resourceSet.clear();
}

RenderContext::RenderContext(rhi::CommandBuffer &cb) : commandBuffer{cb} {
  constexpr auto kNumDescriptorSets = 4;
  resourceSet.reserve(kNumDescriptorSets);
}

} // namespace gfx
