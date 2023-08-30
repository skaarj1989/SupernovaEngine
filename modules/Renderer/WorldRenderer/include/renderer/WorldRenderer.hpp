#pragma once

#include "DebugDraw.hpp"

#include "CubemapConverter.hpp"
#include "IBL.hpp"

#include "DummyResources.hpp"
#include "TransientResources.hpp"

#include "PerspectiveCamera.hpp"
#include "Light.hpp"
#include "MeshInstance.hpp"
#include "DecalInstance.hpp"

#include "TiledLighting.hpp"
#include "ShadowRenderer.hpp"
#include "GlobalIllumination.hpp"

#include "CommonSamplers.hpp"

#include "GBufferPass.hpp"
#include "DecalPass.hpp"
#include "DeferredLightingPass.hpp"
#include "TransparencyPass.hpp"
#include "TransmissionPass.hpp"
#include "WeightedBlendedPass.hpp"
#include "SkyboxPass.hpp"
#include "SSAO.hpp"
#include "SSR.hpp"
#include "Bloom.hpp"
#include "WireframePass.hpp"
#include "DebugNormalPass.hpp"
#include "EyeAdaptation.hpp"
#include "TonemapPass.hpp"
#include "FXAA.hpp"
#include "PostProcessor.hpp"
#include "FinalPass.hpp"
#include "DebugDrawPass.hpp"
#include "InfiniteGridPass.hpp"

#include "Blur.hpp"
#include "Blit.hpp"

#include "SkyLight.hpp"
#include "RenderSettings.hpp"
#include "PipelineGroups.hpp"

namespace gfx {

struct SceneView {
  const std::string name;

  rhi::Texture &target;
  const PerspectiveCamera &camera;
  const RenderSettings &renderSettings;
  SkyLight *skyLight{nullptr};
  std::span<const MaterialInstance> postProcessEffects;
  DebugDraw *debugDraw{nullptr};
};

struct WorldView {
  AABB aabb;
  std::span<const Light *> lights;
  std::span<const MeshInstance *> meshes;
  std::span<const DecalInstance *> decals;
  std::span<const SceneView> sceneViews;
};

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

  [[nodiscard]] uint32_t countPipelines(PipelineGroups) const;
  void clearPipelines(PipelineGroups);

  [[nodiscard]] SkyLight createSkyLight(TextureResourceHandle);

  void drawFrame(rhi::CommandBuffer &, const WorldView &, float deltaTime,
                 DebugOutput * = nullptr);

  [[nodiscard]] std::optional<StageError> isValid(const Material &) const;
  [[nodiscard]] static std::optional<StageError>
  isValid(const rhi::RenderDevice &, const Material &);

private:
  void _drawScene(FrameGraph &, FrameGraphBlackboard, const SceneView &,
                  const Grid &, std::span<const Light *>,
                  const std::vector<Renderable> &renderables,
                  const std::vector<Renderable> &decalRenderables,
                  const PropertyGroupOffsets &, float deltaTime);

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
  GlobalIllumination m_globalIllumination{m_renderDevice, m_commonSamplers};

  GBufferPass m_gBufferPass{m_renderDevice};
  DecalPass m_decalPass{m_renderDevice, m_commonSamplers};
  DeferredLightingPass m_deferredLightingPass{m_renderDevice, m_commonSamplers};

  TransparencyPass m_transparencyPass{m_renderDevice, m_commonSamplers};
  TransmissionPass m_transmissionPass{m_renderDevice, m_commonSamplers};

  SkyboxPass m_skyboxPass{m_renderDevice};

  WeightedBlendedPass m_weightedBlendedPass{m_renderDevice, m_commonSamplers};

  WireframePass m_wireframePass{m_renderDevice};
  DebugNormalPass m_debugNormalPass{m_renderDevice};

  SSAO m_ssao{m_renderDevice, m_commonSamplers};
  SSR m_ssr{m_renderDevice, m_commonSamplers};
  Bloom m_bloom{m_renderDevice, m_commonSamplers};

  EyeAdaptation m_eyeAdaptation{m_renderDevice};
  TonemapPass m_tonemapPass{m_renderDevice, m_commonSamplers};
  FXAA m_fxaa{m_renderDevice, m_commonSamplers};

  PostProcessor m_postProcessor{m_renderDevice, m_commonSamplers};
  FinalPass m_finalPass{m_renderDevice, m_commonSamplers};

  DebugDrawPass m_debugDrawPass{m_renderDevice};
  InfiniteGridPass m_infiniteGridPass{m_renderDevice};

  Blur m_blur{m_renderDevice, m_commonSamplers};
  Blit m_blit{m_renderDevice, m_commonSamplers};
};

} // namespace gfx
