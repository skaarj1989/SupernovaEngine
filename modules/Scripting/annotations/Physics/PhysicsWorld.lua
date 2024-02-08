---@meta

---@class PhysicsWorld
PhysicsWorld = {}

---@param v vec3
function PhysicsWorld:setGravity(v) end

---@return vec3
function PhysicsWorld:getGravity() end

---@class RayCastBPResult
---@field position vec3
---@field entityId integer
RayCastBPResult = {}

---@param from vec3
---@param direction vec3
---@return RayCastBPResult[]
function PhysicsWorld:castRayBP(from, direction) end

---@class RayCastNPResult
---@field position vec3 # world-space
---@field normal vec3 # world-space
---@field entityId integer
RayCastNPResult = {}

---@param from vec3
---@param direction vec3
---@return RayCastNPResult?
function PhysicsWorld:castRayNP(from, direction) end
