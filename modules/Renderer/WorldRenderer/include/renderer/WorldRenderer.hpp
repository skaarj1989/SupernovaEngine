#pragma once

#include "IBL.hpp"

#include "DummyResources.hpp"
#include "TransientResources.hpp"

#include "CommonSamplers.hpp"

#include "TiledLighting.hpp"
#include "ShadowRenderer.hpp"
#include "GlobalIllumination.hpp"

#include "GBufferPass.hpp"
#include "DecalPass.hpp"
#include "DeferredLightingPass.hpp"

#include "TransparencyPass.hpp"
#include "TransmissionPass.hpp"

#include "SkyboxPass.hpp"
#include "WeightedBlendedPass.hpp"

#include "WireframePass.hpp"
#include "DebugNormalPass.hpp"

#include "SSAO.hpp"
#include "SSR.hpp"
#include "Bloom.hpp"

#include "EyeAdaptation.hpp"
#include "TonemapPass.hpp"
#include "FXAA.hpp"

#include "PostProcessor.hpp"
#include "FinalPass.hpp"

#include "DebugDrawPass.hpp"
#include "InfiniteGridPass.hpp"
#include "OutlineRenderer.hpp"

#include "Blur.hpp"
#include "Blit.hpp"

#include "SceneView.hpp"
#include "SkyLight.hpp"

#include "PipelineGroups.hpp"

namespace gfx {

class CubemapConverter;

struct Light;

struct SceneView;
struct WorldView;

struct DebugOutput {
  std::string dot;
};

using StageError = std::map<rhi::ShaderType, std::string>;

class WorldRenderer {
public:
  explicit WorldRenderer(gfx::CubemapConverter &);
  WorldRenderer(const WorldRenderer &) = delete;
  WorldRenderer(WorldRenderer &&) = delete;
  ~WorldRenderer() = default;

  WorldRenderer &operator=(const WorldRenderer &) = delete;
  WorldRenderer &operator=(WorldRenderer &&) = delete;

  [[nodiscard]] rhi::RenderDevice &getRenderDevice() const;

  [[nodiscard]] uint32_t countPipelines(const PipelineGroups) const;
  void clearPipelines(const PipelineGroups);

  [[nodiscard]] SkyLight createSkyLight(TextureResourceHandle);

  void drawFrame(rhi::CommandBuffer &, const WorldView &, const float deltaTime,
                 DebugOutput * = nullptr);

  [[nodiscard]] std::optional<StageError> isValid(const Material &) const;
  [[nodiscard]] static std::optional<StageError>
  isValid(const rhi::RenderDevice &, const Material &);

private:
  void _drawScene(FrameGraph &, FrameGraphBlackboard, const SceneView &,
                  const Grid &, std::span<const Light *>,
                  const std::vector<Renderable> &renderables,
                  const std::vector<Renderable> &decalRenderables,
                  const PropertyGroupOffsets &, const float deltaTime);

private:
  rhi::RenderDevice &m_renderDevice;
  float m_time{0.0f};

  CubemapConverter &m_cubemapConverter;
  IBL m_ibl{m_renderDevice};
  rhi::Texture m_brdf;

  // ---

  DummyResources m_dummyResources{m_renderDevice};
  TransientResources m_transientResources{m_renderDevice};

  CommonSamplers m_commonSamplers;

  TiledLighting m_tiledLighting{m_renderDevice};
  ShadowRenderer m_shadowRenderer{m_renderDevice};
  GlobalIllumination m_globalIllumination{m_renderDevice};

  GBufferPass m_gBufferPass{m_renderDevice};
  DecalPass m_decalPass{m_renderDevice};
  DeferredLightingPass m_deferredLightingPass{m_renderDevice};

  TransparencyPass m_transparencyPass{m_renderDevice};
  TransmissionPass m_transmissionPass{m_renderDevice};

  SkyboxPass m_skyboxPass{m_renderDevice};

  WeightedBlendedPass m_weightedBlendedPass{m_renderDevice};

  WireframePass m_wireframePass{m_renderDevice};
  DebugNormalPass m_debugNormalPass{m_renderDevice};

  SSAO m_ssao{m_renderDevice};
  SSR m_ssr{m_renderDevice};
  Bloom m_bloom{m_renderDevice};

  EyeAdaptation m_eyeAdaptation{m_renderDevice};
  TonemapPass m_tonemapPass{m_renderDevice};
  FXAA m_fxaa{m_renderDevice};

  PostProcessor m_postProcessor{m_renderDevice};
  FinalPass m_finalPass{m_renderDevice};

  DebugDrawPass m_debugDrawPass{m_renderDevice};
  InfiniteGridPass m_infiniteGridPass{m_renderDevice};
  OutlineRenderer m_outlineRenderer{m_renderDevice};

  Blur m_blur{m_renderDevice};
  Blit m_blit{m_renderDevice};
};

} // namespace gfx
