---@meta

---@class RigidBody : ComponentBase, Emitter
---@overload fun(): RigidBody
---@overload fun(settings: RigidBody.Settings): RigidBody
RigidBody = {}

---@class RigidBody.Settings
---@field layer CollisionLayer
---@field isSensor boolean
---@field mass number
---@field motionType MotionType
---@field friction number
---@field restitution number
---@field linearDamping number
---@field angularDamping number
---@field gravityFactor number
RigidBody.Settings = {}

---@enum MotionType
MotionType = {
  Dynamic = 0,
  Static = 1,
  Kinemtaic = 2,
}

---@return RigidBody.Settings
function RigidBody:getSettings() end

---@param v vec3
function RigidBody:setLinearVelocity(v) end

---@return vec3
function RigidBody:getLinearVelocity() end

---@param v vec3
function RigidBody:applyImpulse(v) end
