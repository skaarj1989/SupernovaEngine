--[[
    Main Entity:
    - Transform
    - ColliderComponent
    - CharacterVirtual
    - ScriptComponent (this code)
    + Child Entity:
      - Transform
      + Child Entity:
        - Transform
        - CameraComponent
        - ListenerComponent
]]

local CharacterController = require "Scripts.behavior.CharacterController"
local ShakeableTransform = require "Scripts.behavior.ShakeableTransform"
local HeadBobbing = require "Scripts.behavior.HeadBobbing"
local Cannon = require "Scripts.behavior.Cannon"

local node = {
  lookSpeed = 7,
}

function node:init()
  local character = self.entity:get(CharacterVirtual)
  self.characterController = CharacterController:new(character)

  local head = self.entity:getChildren()[1]

  local headXf <const> = head:get(Transform)
  self.cameraShake = ShakeableTransform:new(headXf)

  local e = head:getChildren()[1]
  self.cameraXf = e:get(Transform)
  self.headBobbing = HeadBobbing:new(headXf, self.cameraXf)

  self.listener = e:get(ListenerComponent)

  self.cannon = Cannon:new({
    speed = 30,
    mass = 0.5,
  })
end

---@param evt InputEvent
function node:input(evt)
  if self.hasControl and isTypeOf(evt, MouseButtonEvent) then
    ---@cast evt MouseButtonEvent
    if evt.state == MouseButtonState.Released and evt.button == MouseButton.Left then
      self:_fire()
    end
  end

  if isTypeOf(evt, KeyboardEvent) then
    ---@cast evt KeyboardEvent
    if evt.state == KeyState.Up then
      if evt.keyCode == KeyCode.R then
        self.cannon:clearSpawns()
      elseif evt.keyCode == KeyCode.C then
        self:_takeControl(not self.hasControl)
      elseif evt.keyCode == KeyCode.Esc then
        self:_takeControl(false)
      end
    else -- KeyDown
      if evt.keyCode == KeyCode.Space then
        self.characterController:jump()
      end
    end
  end
end

---@param dt number
function node:update(dt)
  if self.hasControl then
    self:_processKeyboard()
    self:_processMouse(dt)
  end

  if self.characterController:isMoving() then
    self.headBobbing.isWalking = true
    self.headBobbing.frequency = self.characterController:isWalking() and 6 or 12
  else
    self.headBobbing.isWalking = false
  end

  self.listener.velocity = self.characterController.character:getLinearVelocity()

  self.headBobbing:update(dt)
  self.cameraShake:update(dt)
end

---@param dt number
function node:physicsStep(dt)
  self.characterController:physicsStep(dt)
end

---@param b boolean
function node:_takeControl(b)
  self.hasControl = b
  InputSystem:showCursor(not self.hasControl)
end

function node:_processKeyboard()
  local moveDirection = math.vec3()
  if InputSystem:isKeyDown(KeyCode.W) then
    moveDirection.z = moveDirection.z + 1
  end
  if InputSystem:isKeyDown(KeyCode.S) then
    moveDirection.z = moveDirection.z - 1
  end
  if InputSystem:isKeyDown(KeyCode.A) then
    moveDirection.x = moveDirection.x + 1
  end
  if InputSystem:isKeyDown(KeyCode.D) then
    moveDirection.x = moveDirection.x - 1
  end
  self.characterController:setMoveDirection(moveDirection)
  self.characterController:sprint(InputSystem:isKeyDown(KeyCode.Shift))
end

---@param dt number
function node:_processMouse(dt)
  local mouseDelta <const> = InputSystem:getMouseDelta()
  if (mouseDelta ~= math.ivec2(0)) then
    -- Horizontal axis: rotate character body around Y axis.
    local qYaw = math.angleAxis(math.radians(-mouseDelta.x * self.lookSpeed * dt), Transform.Up)
    self.characterController:setYaw(qYaw)

    -- Vertical axis: rotate camera (up/down).
    local qPitch = math.angleAxis(math.radians(mouseDelta.y * self.lookSpeed * dt), Transform.Right)
    local eulerAngles = math.eulerAngles(self.cameraXf:getLocalOrientation() * qPitch)
    local maxAngle <const> = 1.4 -- in radians (1.4 = ~80deg)
    eulerAngles.x = math.clamp(eulerAngles.x, -maxAngle, maxAngle)
    self.cameraXf:setOrientation(math.quat(eulerAngles))
  end

  -- Lock the mouse cursor in the center of the GameWindow.
  InputSystem:setMousePosition(math.ivec2(getCenter(GameWindow)))
end

function node:_fire()
  local cameraForward = self.cameraXf:getForward()
  local offset <const> = 4
  local spawnPos = self.cameraXf:getPosition() + (cameraForward * offset)
  self.cannon:fire(createEntity(), spawnPos, cameraForward)
  self.cameraShake:induceStress(0.15)
end

return node
