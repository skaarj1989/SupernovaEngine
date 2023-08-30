--- @meta

--- @class Light : ComponentBase
--- @field type LightType
--- @field visible boolean
--- @field castsShadow boolean
--- @field debugVolume boolean
--- @field color vec3
--- @field intensity number
--- @field range number
--- @field innerConeAngle number
--- @field outerConeAngle number
--- @field shadowBias number
Light = {}

--- @enum LightType
LightType = {
    Directional = 0,
    Spot = 1,
    Point = 2,
}
