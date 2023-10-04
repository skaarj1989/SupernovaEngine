local class = require "middleclass"

local CharacterController = class("CharacterController")

local State <const> = {
  IDLE = 0,
  WALKING = 1,
  RUNNING = 2,
  IN_AIR = 3,
}

---@param character CharacterVirtual
function CharacterController:construct(character)
  self.character = character
  self.state = State.IDLE

  self.moveDirection = math.vec3()
  self.desiredVelocity = math.vec3()

  self.wantJump = false
  self.wantSprint = false

  self.walkSpeed = 3.0
  self.sprintMultiplier = 2.0
  self.jumpSpeed = 5.0
end

---@param dt number
function CharacterController:physicsStep(dt)
  local controlMovementDuringJump <const> = false

  local playerControlsHorizontalVelocity = controlMovementDuringJump or self.character:isSupported()
  if playerControlsHorizontalVelocity then
    -- Smooth the player input.
    local speed <const> = self.walkSpeed * (self.wantSprint and self.sprintMultiplier or 1.0)
    self.desiredVelocity = 0.25 * self.moveDirection * speed + 0.75 * self.desiredVelocity
    self.allowSliding = self.moveDirection ~= math.vec3(0)
  else
    -- While in air we allow sliding.
    self.allowSliding = true
  end

  -- Determine new basic velocity:

  local currentVelocity <const> = self.character:getLinearVelocity()
  local currentVerticalVelocity <const> = math.dot(currentVelocity, Transform.Up) * Transform.Up --[[@as vec3]]
  local groundVelocity <const> = self.character:getGroundVelocity()
  local movingTowardsGround <const> = (currentVerticalVelocity.y - groundVelocity.y) < 0.1
  local currentHorizontalVelocity <const> = currentVelocity - currentVerticalVelocity

  local newVelocity = nil

  if self.character:getGroundState() == GroundState.OnGround and movingTowardsGround then
    -- Assume velocity of ground when no ground.
    newVelocity = groundVelocity

    if math.length(self.moveDirection) ^ 2 > math.EPSILON() ^ 2 then
      self.state = self.wantSprint and State.RUNNING or State.WALKING
    else
      self.state = State.IDLE
    end

    -- Jump:
    if self.wantJump and movingTowardsGround then
      newVelocity = newVelocity + (self.jumpSpeed * Transform.Up)
      self.state = State.IN_AIR
    end
  else
    newVelocity = currentVerticalVelocity
    self.state = State.IN_AIR
  end

  -- Gravity.
  local gravity <const> = math.vec3(0, -9.8, 0)
  newVelocity = newVelocity + (gravity * dt)

  if playerControlsHorizontalVelocity then
    -- Player input.
    newVelocity = newVelocity + self.desiredVelocity
  else
    -- Preserve horizontal velocity (+ slight control in the air).
    newVelocity = newVelocity + currentHorizontalVelocity + self.moveDirection * 0.1
    local maxSpeed <const> = self.walkSpeed * self.sprintMultiplier
    newVelocity.x = math.clamp(newVelocity.x, -maxSpeed, maxSpeed)
    newVelocity.z = math.clamp(newVelocity.z, -maxSpeed, maxSpeed)
  end

  self.character:setLinearVelocity(newVelocity)

  self.wantJump = false
end

function CharacterController:isMoving()
  return self:isWalking() or self:isRunning()
end

function CharacterController:isWalking()
  return self.state == State.WALKING
end

function CharacterController:isRunning()
  return self.state == State.RUNNING
end

---@param moveDirection vec3
function CharacterController:setMoveDirection(moveDirection)
  if math.length(moveDirection) ~= 0 then
    moveDirection = self.character:getRotation() * math.normalize(moveDirection)
  end
  self.moveDirection = moveDirection
end

---@param q quat
function CharacterController:setYaw(q)
  self.character:setRotation(self.character:getRotation() * q)
end

---@param b boolean
function CharacterController:sprint(b)
  self.wantSprint = b
end

function CharacterController:jump()
  self.wantJump = true
end

return CharacterController
