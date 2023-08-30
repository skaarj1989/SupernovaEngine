#include "renderer/DummyResources.hpp"

#include "fg/FrameGraph.hpp"
#include "renderer/FrameGraphBuffer.hpp"
#include "FrameGraphImport.hpp"
#include "fg/Blackboard.hpp"

#include "FrameGraphData/DummyResources.hpp"

namespace gfx {

DummyResources::DummyResources(rhi::RenderDevice &rd) {
  m_uniformBuffer = rd.createUniformBuffer(1, rhi::AllocationHints::MinMemory);
  m_storageBuffer = rd.createStorageBuffer(1, rhi::AllocationHints::MinMemory);

  rhi::Texture::Builder builder{};
  builder.setExtent({1, 1})
    .setPixelFormat(rhi::PixelFormat::R8_UNorm)
    .setNumMipLevels(1)
    .setNumLayers(std::nullopt)
    .setUsageFlags(rhi::ImageUsage::Sampled)
    .setupOptimalSampler(true);

  m_texture2D = builder.setCubemap(false).build(rd);
  m_cubeMap = builder.setCubemap(true).build(rd);

  builder.setPixelFormat(rhi::PixelFormat::Depth16)
    .setNumLayers(1)
    .setupOptimalSampler(false);

  m_shadowMaps2DArray = builder.setCubemap(false).build(rd);
  m_shadowMapsCubeArray = builder.setCubemap(true).build(rd);
}

void DummyResources::embedDummyResources(FrameGraph &fg,
                                         FrameGraphBlackboard &blackboard) {
  blackboard.add<DummyResourcesData>() = {
    .uniformBuffer =
      fg.import <FrameGraphBuffer>("DummyUB",
                                   {
                                     .type = BufferType::UniformBuffer,
                                     .capacity = m_uniformBuffer.getSize(),
                                   },
                                   {&m_uniformBuffer}),
    .storageBuffer =
      fg.import <FrameGraphBuffer>("DummySB",
                                   {
                                     .type = BufferType::StorageBuffer,
                                     .capacity = m_storageBuffer.getSize(),
                                   },
                                   {&m_storageBuffer}),

    .texture2D = importTexture(fg, "DummyTexture2D", &m_texture2D),
    .cubeMap = importTexture(fg, "DummyCubeMap", &m_cubeMap),
    .shadowMaps2DArray =
      importTexture(fg, "DummyShadowMap2DArray", &m_shadowMaps2DArray),
    .shadowMapsCubeArray =
      importTexture(fg, "DummyShadowMapCubeArray", &m_shadowMapsCubeArray),
  };
}

} // namespace gfx
