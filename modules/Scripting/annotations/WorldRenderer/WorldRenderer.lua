---@meta

---@class RenderSettings
---@field outputMode OutputMode
---@field features RenderFeatures
---@field ambientLight vec4
---@field IBLIntensity number
---@field globalIllumination GlobalIllumination
---@field shadow ShadowSettings
---@field ssao SSAOSettings
---@field bloom Bloom
---@field exposure number
---@field adaptiveExposure AdaptiveExposure
---@field tonemap Tonemap
---@field debugFlags DebugFlags
RenderSettings = {}

---@enum OutputMode
OutputMode = {
  Depth = 0,
  Normal = 1,
  Emissive = 2,
  BaseColor = 3,
  Metallic = 4,
  Roughness = 5,
  AmbientOcclusion = 6,

  SSAO = 7,
  BrightColor = 8,
  Reflections = 9,

  Accum = 10,
  Reveal = 11,

  LightHeatmap = 12,

  HDR = 13,
  FinalImage = 14,
}

---@enum RenderFeatures
RenderFeatures = {
  None = 0,

  LightCulling = 1 << 0,
  SoftShadows = 1 << 1,
  GI = 1 << 2,
  SSAO = 1 << 3,
  SSR = 1 << 4,
  Bloom = 1 << 5,
  FXAA = 1 << 6,
  EyeAdaptation = 1 << 7,
  CustomPostprocess = 1 << 8,

  Default = RenderFeatures.LightCulling | RenderFeatures.SSAO | RenderFeatures.Bloom | RenderFeatures.FXAA |
      RenderFeatures.CustomPostprocess,

  All = RenderFeatures.Default | RenderFeatures.SoftShadows | RenderFeatures.GI | RenderFeatures.SSR,
}

---@class GlobalIllumination
---@field numPropagations integer
---@field intensity number
GlobalIllumination = {}

---@class ShadowSettings
---@field cascadedShadowMaps CascadedShadowMaps
---@field spotLightShadowMaps SpotLightShadowMaps
---@field omniShadowMaps OmniShadowMaps
ShadowSettings = {
  ---@class CascadedShadowMaps
  ---@field numCascades integer
  ---@field shadowMapSize integer
  ---@field lambda number
  CascadedShadowMaps = {},

  ---@class SpotLightShadowMaps
  ---@field maxNumShadows integer
  ---@field shadowMapSize integer
  SpotLightShadowMaps = {},

  ---@class OmniShadowMaps
  ---@field maxNumShadows integer
  ---@field shadowMapSize integer
  OmniShadowMaps = {}
}

---@class SSAOSettings
---@field radius number
---@field power number
---@field bias number
SSAOSettings = {}

---@class Bloom
---@field radius number
---@field strength number
Bloom = {}

---@class AdaptiveExposure
---@field minLogLuminance number
---@field maxLogLuminance number
---@field tau number
AdaptiveExposure = {}

---@enum Tonemap
Tonemap = {
  Clamp = 0,
  ACES = 1,
  Filmic = 2,
  Reinhard = 3,
  Uncharted = 4,
}

---@enum DebugFlags
DebugFlags = {
  None = 0,

  WorldBounds = 1 << 0,
  InfiniteGrid = 1 << 1,

  Wireframe = 1 << 2,
  VertexNormal = 1 << 3,

  CascadeSplits = 1 << 4,
  LightHeatmap = 1 << 5,

  VPL = 1 << 6,
  RadianceOnly = 1 << 7,
}
