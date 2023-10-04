local class = require "middleclass"

local HeadBobbing = class("HeadBobbing")

---@param headXf Transform
---@param cameraXf Transform
function HeadBobbing:construct(headXf, cameraXf)
  self.headXf = headXf
  self.cameraXf = cameraXf

  self.frequency = 6.0
  self.horizontalAmplitude = 0.1
  self.verticalAmplitude = 0.1
  self.smoothing = 0.15

  self.isWalking = false
  self.walkingTime = 0.0
end

---@param dt number
function HeadBobbing:update(dt)
  if self.isWalking then
    self.walkingTime = self.walkingTime + dt
  else
    self.walkingTime = 0.0
  end

  local cameraPosition <const> = self.cameraXf:getLocalPosition()
  local offset <const> = self:_calculateHeadBobOffset(self.walkingTime)
  local targetCameraPosition = math.mix(cameraPosition, offset, math.vec3(self.smoothing))
  if math.length(targetCameraPosition - offset) <= 0.001 then
    targetCameraPosition = offset
  end
  self.cameraXf:setPosition(targetCameraPosition)
end

---@param t number
function HeadBobbing:_calculateHeadBobOffset(t)
  local offset = math.vec3(0.0)
  if t > 0.0 then
    local horizontalOffset <const> = math.cos(t * self.frequency) * self.horizontalAmplitude
    local verticalOffset <const> = math.sin(t * self.frequency * 2.0) * self.verticalAmplitude
    offset = self.headXf:getRight() * horizontalOffset + self.headXf:getUp() * verticalOffset
  end
  return offset
end

return HeadBobbing
