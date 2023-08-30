--- @meta

--- @enum MaterialDomain
MaterialDomain = {
    Surface = 0, PostProcess = 1,
}

--- @enum ShadingModel
ShadingModel = {
    Unlit = 0, Lit = 1
}

--- @enum BlendMode
BlendMode = {
    Opaque = 0, Masked = 1, Transparent = 2, Add = 3, Modulate = 4
}

--- @enum LightingMode
LightingMode = {
    Default = 0, Transmission = 1
}

--- @class MaterialSurface
--- @field shadingModel ShadingModel
--- @field blendMode BlendMode
--- @field lightingMode LightingMode
--- @field cullMode CullMode
MaterialSurface = {}

--- @class MaterialBlueprint
--- @field surface MaterialSurface
MaterialBlueprint = {}

--- @class Material
Material = {}

--- @return string
function Material:getName() end

--- @return MaterialBlueprint
function Material:getBlueprint() end

--- @return MaterialDomain
--- @param blueprint MaterialBlueprint
function getDomain(blueprint) end
