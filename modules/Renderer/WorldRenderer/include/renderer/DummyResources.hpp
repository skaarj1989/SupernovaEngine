#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderDevice.hpp"

namespace gfx {

class DummyResources final {
public:
  explicit DummyResources(rhi::RenderDevice &);

  void embedDummyResources(FrameGraph &, FrameGraphBlackboard &);

private:
  rhi::UniformBuffer m_uniformBuffer;
  rhi::StorageBuffer m_storageBuffer;

  rhi::Texture m_texture2D;
  rhi::Texture m_cubeMap;
  rhi::Texture m_shadowMaps2DArray;
  rhi::Texture m_shadowMapsCubeArray;
};

} // namespace gfx
