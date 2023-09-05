--[[
    Main Entity:
    - Transform
    - ColliderComponent
    - CharacterVirtual
    - ScriptComponent (this code)

    Child Entity:
    - Transform
    - CameraComponent
]]
local bulletPrefab <const> = {
  collider = loadCollider("Colliders/Sphere.json"),
  mesh = loadMesh("BasicShapes:Sphere"),
  material = loadMaterial("Materials/Graph/Surface/Emissive/Emissive.material")
}

math.randomseed(os.time())

local function getRandomColor()
  return math.vec3(math.random(), math.random(), math.random())
end

--- @class Player: ScriptNode
local node = {
  walkSpeed = 6,
  sprintMultiplier = 2.5,
  lookSpeed = 7,
  jumpSpeed = 4,

  projectile = {
    speed = 30,
    mass = 0.5,
  },

  hasControl = false,

  moveDirection = math.vec3(0),
  desiredVelocity = math.vec3(0),
  allowSliding = false,
  wantJump = false,

  spawns = {},
}

function node:input(evt)
  if self.hasControl and isTypeOf(evt, MouseButtonEvent) then
    --- @cast evt MouseButtonEvent
    if evt.state == MouseButtonState.Released and evt.button == MouseButton.Left then
      self:_fire()
    end
  end

  if isTypeOf(evt, KeyboardEvent) then
    --- @cast evt KeyboardEvent
    if evt.state == KeyState.Up then
      if evt.keyCode == KeyCode.R then
        self:_clearSpawns()
      elseif evt.keyCode == KeyCode.C then
        self.hasControl = not self.hasControl
        InputSystem:showCursor(not self.hasControl)
      end
    else -- KeyDown
      if evt.keyCode == KeyCode.Space then
        self.wantJump = true
      end
    end
  end
end

function node:_fire()
  local bullet <const> = createEntity()

  local cameraForward = self.cameraXf:getForward()
  local offset = 4
  local spawnPos = self.cameraXf:getPosition() + (cameraForward * offset)
  bullet:emplace(Transform(spawnPos))

  bullet:emplace(ColliderComponent(bulletPrefab.collider))
  local body = bullet:emplace(RigidBody(RigidBodySettings({
    motionType = MotionType.Dynamic,
    mass = self.projectile.mass,
    restitution = 0.6,
  })))
  body:setLinearVelocity(cameraForward * self.projectile.speed)

  local color <const> = getRandomColor()
  bullet:emplace(Light({
    type = LightType.Point,
    color = color,
    range = 10,
    intensity = 30,
    shadowBias = 0.035,
    castsShadow = true
  }))

  local meshInstance = bullet:emplace(MeshInstance(bulletPrefab.mesh))
  meshInstance:setMaterial(0, bulletPrefab.material)
  local materialInstance = meshInstance:getMaterial(0)
  materialInstance.receivesShadow = false
  materialInstance.castsShadow = false
  local intensity <const> = 500
  materialInstance:setProperty("fs_emissiveFactor", math.vec4(color, intensity))

  table.insert(self.spawns, bullet)
end

function node:_clearSpawns()
  for i = 1, #self.spawns do
    self.spawns[i]:destroy()
  end
  self.spawns = {}
end

function node:_processMouse(dt)
  local mouseDelta = InputSystem:getMouseDelta()
  if (mouseDelta ~= math.ivec2(0)) then
    -- Horizontal axis: rotate character body around Y axis.
    local qYaw = math.angleAxis(math.radians(-mouseDelta.x * self.lookSpeed * dt), Transform.Up)
    self.character:setRotation(self.character:getRotation() * qYaw)

    -- Vertical axis: rotate camera (up/down).
    local qPitch = math.angleAxis(math.radians(mouseDelta.y * self.lookSpeed * dt), Transform.Right)
    self.cameraXf:rotate(qPitch)
  end

  -- Lock the mouse cursor in the center of the GameWindow.
  InputSystem:setMousePosition(math.ivec2(getCenter(GameWindow)))
end

function node:_processKeyboard(dt)
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

  if math.length(moveDirection) ~= 0 then
    moveDirection = self.character:getRotation() * math.normalize(moveDirection)
  end

  self.moveDirection = moveDirection
end

function node:init()
  self.xf        = self.entity:get(Transform)
  self.character = self.entity:get(CharacterVirtual)

  local children = self.entity:getChildren()
  self.cameraXf  = children[1]:get(Transform)
end

function node:update(dt)
  if self.hasControl then
    self:_processKeyboard(dt)
    self:_processMouse(dt)
  end
end

function node:_getCharacterSpeed()
  return self.walkSpeed * ((InputSystem:isKeyDown(KeyCode.Shift) and self.sprintMultiplier) or 1.0)
end

function node:physicsStep(dt)
  local controlMovementDuringJump <const> = true

  local playerControlsHorizontalVelocity = controlMovementDuringJump or self.character:isSupported()
  if playerControlsHorizontalVelocity then
    -- Smooth the player input.
    self.desiredVelocity = 0.25 * self.moveDirection * self:_getCharacterSpeed() + 0.75 * self.desiredVelocity
    self.allowSliding = self.moveDirection ~= math.vec3(0)
  else
    -- While in air we allow sliding.
    self.allowSliding = true
  end

  -- Determine new basic velocity:

  local up <const> = self.character:getUp()

  local currentVelocity = self.character:getLinearVelocity()
  local currentVerticalVelocity = math.dot(currentVelocity, up) * up --[[@as vec3]]
  local groundVelocity = self.character:getGroundVelocity()
  local movingTowardsGround = (currentVerticalVelocity.y - groundVelocity.y) < 0.1

  local newVelocity = math.vec3()

  if self.character:getGroundState() == GroundState.OnGround and movingTowardsGround then
    -- Assume velocity of ground when no ground.
    newVelocity = groundVelocity

    -- Jump:
    if self.wantJump and movingTowardsGround then
      newVelocity = newVelocity + (self.jumpSpeed * up)
    end
  else
    newVelocity = currentVerticalVelocity
  end

  -- Gravity.
  local gravity <const> = math.vec3(0, -9.8, 0)
  newVelocity = newVelocity + (gravity * dt)

  if playerControlsHorizontalVelocity then
    -- Player input.
    newVelocity = newVelocity + self.desiredVelocity
  else
    -- Preserve horizontal velocity.
    local currentHorizontalVelocity = currentVelocity - currentVerticalVelocity;
    newVelocity = newVelocity + currentHorizontalVelocity
  end

  self.character:setLinearVelocity(newVelocity)

  self.wantJump = false
end

return node
