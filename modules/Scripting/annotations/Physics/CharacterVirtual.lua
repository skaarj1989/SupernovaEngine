--- @meta

--- @class CharacterVirtual : ComponentBase
--- @field stickToFloor boolean
--- @field walkStairs boolean
--- @overload fun(): CharacterVirtual
--- @overload fun(settings: CharacterVirtualSettings): CharacterVirtual
CharacterVirtual = {}

--- @class CharacterVirtualSettings
--- @field maxSlopeAngle number In degrees
--- @field layer CollisionLayer
--- @field mass number
--- @field maxStrength number
--- @field shapeOffset vec3
--- @field predictiveContactDistance number
--- @field maxCollisionIterations integer
--- @field maxConstraintIterations integer
--- @field minTimeRemaining number
--- @field collisionTolerance number
--- @field characterPadding number
--- @field maxNumHits integer
--- @field hitReductionCosMaxAngle number
--- @field penetrationRecoverySpeed number
CharacterVirtualSettings = {}

--- @return CharacterVirtualSettings
function CharacterVirtual:getSettings() end

--- @param q quat
function CharacterVirtual:setRotation(q) end

--- @param v vec3
function CharacterVirtual:setLinearVelocity(v) end

--- @return vec3
function CharacterVirtual:getUp() end

--- @return vec3
function CharacterVirtual:getPosition() end

--- @return quat
function CharacterVirtual:getRotation() end

--- @return vec3
function CharacterVirtual:getLinearVelocity() end

--- @return GroundState
function CharacterVirtual:getGroundState() end

--- @return boolean
function CharacterVirtual:isSupported() end

--- @return vec3
function CharacterVirtual:getGroundNormal() end

--- @return vec3
function CharacterVirtual:getGroundVelocity() end
