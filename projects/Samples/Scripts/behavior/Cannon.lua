local class = require "middleclass"

local Cannon = class("Cannon")

function Cannon:construct(projectile)
  math.randomseed(os.time())

  self.projectile = projectile
  self.spawns = {}
end

local bulletPrefab <const> = {
  collider = loadCollider("Colliders/Sphere"),
  mesh = loadMesh("BasicShapes:Sphere"),
  material = loadMaterial("Materials/Graph/Surface/Emissive/Emissive.material")
}
local function getRandomColor()
  return math.vec3(math.random(), math.random(), math.random())
end

---@param bullet Entity
---@param position vec3
---@param direction vec3
function Cannon:fire(bullet, position, direction)
  bullet:emplace(Transform(position))
  bullet:emplace(ColliderComponent(bulletPrefab.collider))
  local body = bullet:emplace(RigidBody(RigidBodySettings({
    motionType = MotionType.Dynamic,
    mass = self.projectile.mass,
    restitution = 0.6,
  })))
  body:setLinearVelocity(direction * self.projectile.speed)

  local color <const> = getRandomColor()
  bullet:emplace(Light({
    type = LightType.Point,
    color = color,
    range = 10,
    intensity = 30,
    shadowBias = 0.06,
    castsShadow = true
  }))

  local meshInstance = bullet:emplace(MeshInstance(bulletPrefab.mesh))
  meshInstance:setMaterial(0, bulletPrefab.material)
  local materialInstance = meshInstance:getMaterial(0)
  materialInstance.receivesShadow = false
  materialInstance.castsShadow = true
  local intensity <const> = 500
  materialInstance:setProperty("fs_emissiveFactor", math.vec4(color, intensity))

  table.insert(self.spawns, bullet)
end

function Cannon:clearSpawns()
  for i = 1, #self.spawns do
    self.spawns[i]:destroy()
  end
  self.spawns = {}
end

return Cannon
