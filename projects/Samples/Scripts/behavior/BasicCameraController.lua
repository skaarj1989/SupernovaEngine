---@class BasicCameraController : ScriptNode
local node = {
  captureMouse = false,

  moveSpeed = 20,
  lookSpeed = 7,
}

function node:init()
  self.xf = self.entity:get(Transform)
end

---@param evt InputEvent
function node:input(evt)
  if isTypeOf(evt, KeyboardEvent) then
    ---@cast evt KeyboardEvent
    if evt.state == KeyState.Up then
      if evt.keyCode == KeyCode.C then
        self.capture = not self.capture
        InputSystem:showCursor(not self.capture)
      end
    end
  end
end

---@param dt number
function node:_processMouse(dt)
  local mouseDelta <const> = InputSystem:getMouseDelta()
  if (mouseDelta ~= math.ivec2(0)) then
    local q <const> = self.xf:getOrientation()
    local qPitch <const> = math.angleAxis(math.radians(mouseDelta.y * self.lookSpeed * dt), Transform.Right)
    local qYaw <const> = math.angleAxis(math.radians(-mouseDelta.x * self.lookSpeed * dt), Transform.Up)

    self.xf:setOrientation(qYaw * q * qPitch)
  end

  InputSystem:setMousePosition(math.ivec2(getCenter(GameWindow)))
end

---@param dt number
function node:_processKeyboard(dt)
  local input = math.vec3()

  if InputSystem:isKeyDown(KeyCode.W) then
    input.z = input.z + 1
  end
  if InputSystem:isKeyDown(KeyCode.S) then
    input.z = input.z - 1
  end
  if InputSystem:isKeyDown(KeyCode.A) then
    input.x = input.x + 1
  end
  if InputSystem:isKeyDown(KeyCode.D) then
    input.x = input.x - 1
  end
  if InputSystem:isKeyDown(KeyCode.Q) then
    input.y = input.y + 1
  end
  if InputSystem:isKeyDown(KeyCode.E) then
    input.y = input.y - 1
  end

  if math.length(input) ~= 0 then
    input = math.normalize(input) --[[@as vec3]]
    
    local q <const> = self.xf:getOrientation()
    local dir <const> = q * input
    local velocity <const> = dir * (self.moveSpeed * (InputSystem:isKeyDown(KeyCode.Shift) and 2 or 1)) * dt
    self.xf:translate(velocity)
    --self.xf:setPosition(self.xf:getPosition() + velocity)
  end
end

function node:update(dt)
  if self.capture then
    self:_processKeyboard(dt)
    self:_processMouse(dt)
  end
end

return node
