---@meta

---@class AABB
---@field min vec3
---@field max vec3
---@overload fun(min: vec3, max: vec3): AABB
math.AABB = {}

---@return vec3
function math.AABB:getCenter() end

---@return vec3
function math.AABB:getExtent() end

---@return number
function math.AABB:getRadius() end

---@param m mat4
---@return AABB
function math.AABB:transform(m) end
