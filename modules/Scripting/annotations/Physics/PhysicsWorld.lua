---@meta

---@class PhysicsWorld
PhysicsWorld = {}

---@enum PhysicsWorld.DebugDrawFlags
PhysicsWorld.DebugDrawFlags = {
    None = 0,
    Shape = 1 << 0,
    BoundingBox = 1 << 1,
    WorldTransform = 1 << 2,
}

---@param flags PhysicsWorld.DebugDrawFlags
function PhysicsWorld:setDebugDrawFlags(flags) end

---@return PhysicsWorld.DebugDrawFlags
function PhysicsWorld:getDebugDrawFlags() end

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
