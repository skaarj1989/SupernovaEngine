local class = require "middleclass"

-- https://github.com/IronWarrior/UnityCameraShake

local ShakeableTransform = class("ShakeableTransform")

--- @param xf Transform
function ShakeableTransform:construct(xf)
  self.xf = xf

  math.randomseed(os.time())
  self.seed = math.random()

  self.traumaExponent = 1.0
  self.frequency = 25
  self.recoverySpeed = 1.0

  self.totalTime = 0.0
  self.trauma = 0.0
end

--- @param dt number
function ShakeableTransform:update(dt)
  self.totalTime = self.totalTime + dt

  if self.trauma <= 0.0 then return end

  local shake <const> = self.trauma ^ self.traumaExponent

  local maxTranslationShake <const> = math.vec3(1) * 0.15
  self.xf:translate(
    math.vec3(
      maxTranslationShake.x * (math.noise(self.seed, self.totalTime * self.frequency)),
      maxTranslationShake.y * (math.noise(self.seed + 1, self.totalTime * self.frequency)),
      maxTranslationShake.z * (math.noise(self.seed + 2, self.totalTime * self.frequency))
    ) * shake
  )

  local maxAngularShake <const> = math.radians(math.vec3(15, 15, 10)) --[[@as vec3]]
  self.xf:rotate(
    math.quat(
      math.vec3(
        maxAngularShake.x * (math.noise(self.seed + 3, self.totalTime * self.frequency)),
        maxAngularShake.y * (math.noise(self.seed + 4, self.totalTime * self.frequency)),
        maxAngularShake.z * (math.noise(self.seed + 5, self.totalTime * self.frequency))
      ) * shake
    )
  )

  self.trauma = math.clamp(self.trauma - self.recoverySpeed * dt, 0.0, 1.0)
end

--- @param stress number
function ShakeableTransform:induceStress(stress)
  self.trauma = math.clamp(self.trauma + stress, 0.0, 1.0)
end

return ShakeableTransform
