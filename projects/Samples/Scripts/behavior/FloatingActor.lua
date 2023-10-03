---@class FloatingActor : ScriptNode
local node = {}

function node:init()
  self.creationTime = os.clock()
  self.xf = self.entity:get(Transform)
end

function node:update(dt)
  local runningTime <const> = os.clock() - self.creationTime

  local deltaHeight <const> = math.sin(runningTime + dt) - math.sin(runningTime)
  self.xf:translate(math.vec3(0, deltaHeight * 1.25, 0))

  local degPerSec <const> = 60
  self.xf:yaw(math.radians(degPerSec * dt))
end

return node
