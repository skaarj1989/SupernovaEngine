--[[
    Main Entity:
    - Transform
    - ColliderComponent
    - Character
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
  hasControl = false,

  moveDirection = math.vec3(),
  walkSpeed = 6,
  sprintMultiplier = 2.5,

  wantJump = true,
  jumpSpeed = 4.0,

  lookSpeed = 7,

  bulletSpeed = 20,

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
    restitution = 0.6,
  })))
  body:setLinearVelocity(cameraForward * self.bulletSpeed)

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
  self.character = self.entity:get(Character)

  local children = self.entity:getChildren()
  self.cameraXf  = children[1]:get(Transform)
end

function node:update(dt)
  if self.hasControl then
    self:_processKeyboard(dt)
    self:_processMouse(dt)
  end
end

function node:physicsStep(dt)
  -- Cancel movement in the opposite direction of normal when sliding:
  local groundState <const> = self.character:getGroundState()
  if groundState == GroundState.Sliding then
    -- TOOD
  end

  -- Update velocity:

  local currentVelocity = self.character:getLinearVelocity()

  local characterSpeed = self.walkSpeed * ((InputSystem:isKeyDown(KeyCode.Shift) and self.sprintMultiplier) or 1.0)
  local desiredVelocity = self.moveDirection * characterSpeed
  desiredVelocity.y = currentVelocity.y

  local newVelocity = (0.75 * currentVelocity) + (0.25 * desiredVelocity)

  -- Jump:

  if self.wantJump and groundState == GroundState.OnGround then
    newVelocity = newVelocity + math.vec3(0, self.jumpSpeed, 0)
    self.wantJump = false
  end

  self.character:setLinearVelocity(newVelocity)
end

return node
