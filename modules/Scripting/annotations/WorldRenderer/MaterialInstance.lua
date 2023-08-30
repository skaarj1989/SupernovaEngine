--- @meta

--- @class MaterialInstance
--- @field castsShadow boolean Surface only
--- @field receivesShadow boolean Surface only
--- @field enabled boolean Postprocess only
--- @overload fun(resource: MaterialResource): MaterialInstance
MaterialInstance = {}

--- @return MaterialResource
function MaterialInstance:getResource() end

--- @return boolean
function MaterialInstance:hasProperties() end

--- @return boolean
function MaterialInstance:hasTextures() end

--- @alias PropertyValue integer|number|vec2|vec4

--- @enum Numeric
Numeric = { Int = 0, Uint = 1, Flaot = 2}

--- @param name string
--- @param value PropertyValue
--- @param type? Numeric
--- @return MaterialInstance self
function MaterialInstance:setProperty(name, value, type) end

--- @param name string
--- @return PropertyValue
function MaterialInstance:getProperty(name) end

--- @param name string
--- @param texture Texture
--- @return MaterialInstance self
function MaterialInstance:setTexture(name, texture) end

--- @return MaterialInstance self
function MaterialInstance:reset() end
