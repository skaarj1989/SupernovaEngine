---@meta

---@class Character : ComponentBase, Emitter
---@overload fun(): Character
---@overload fun(settings: CharacterSettings): Character
Character = {}

---@class CharacterSettings
---@field maxSlopeAngle number # In degrees
---@field layer CollisionLayer
---@field mass number
---@field friction number
---@field gravityFactor number
CharacterSettings = {}

---@return CharacterSettings
function Character:getSettings() end

---@param q quat
function Character:setRotation(q) end

---@param v vec3
function Character:setLinearVelocity(v) end

---@return vec3
function Character:getPosition() end

---@return quat
function Character:getRotation() end

---@return vec3
function Character:getLinearVelocity() end

---@enum GroundState
GroundState = {
  OnGround = 0,
  OnSteepGround = 1,
  NotSupported = 2,
  InAir = 3,
}

---@return GroundState
function Character:getGroundState() end

---@return boolean
function Character:isSupported() end

---@return vec3
function Character:getGroundNormal() end

---@return vec3
function Character:getGroundVelocity() end
