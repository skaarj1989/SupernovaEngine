--- @meta

--- @class RigidBody : ComponentBase
--- @overload fun(): RigidBody
--- @overload fun(settings: RigidBodySettings): RigidBody
RigidBody = {}

--- @class RigidBodySettings
--- @field layer CollisionLayer
--- @field mass number
--- @field motionType MotionType
--- @field friction number
--- @field restitution number
--- @field linearDamping number
--- @field angularDamping number
--- @field gravityFactor number
RigidBodySettings = {}

--- @enum MotionType
MotionType = {
    Dynamic = 0,
    Static = 1,
    Kinemtaic = 2,
}

--- @return RigidBodySettings
function RigidBody:getSettings() end

--- @param v vec3
function RigidBody:setLinearVelocity(v) end

--- @return vec3
function RigidBody:getLinearVelocity() end

--- @param v vec3
function RigidBody:applyImpulse(v) end
