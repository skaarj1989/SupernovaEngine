---@meta

---@class CameraComponent : ComponentBase
---@field extent Extent2D
---@field target Texture # Read only
---@field camera PerspectiveCamera
---@field renderSettings RenderSettings
---@field skyLight SkyLight
---@field postProcessEffects MaterialInstance[]
---@field debugDraw DebugDraw
---@overload fun(): CameraComponent
---@overload fun(extent: Extent2D): CameraComponent
CameraComponent = {}
