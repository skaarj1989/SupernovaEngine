local prefab <const> = {
  mesh = loadMesh("BasicShapes:Cube"),
  collider = loadCollider("Colliders/Box")
}

-- Config:

local startPosition <const> = math.vec3(0)

local boxSize <const> = 0.5
local stackSize = 12
local space <const> = 0.0001

-- Create a pyramid:

local diff <const> = math.vec3(boxSize) * 1.02
local offset = math.vec3()
local pos = math.vec3(0.0, boxSize, 0.0)

while stackSize >= 0 do
  for j = 0, stackSize do
    pos.z = offset.z + j * (diff.z * 2.0 + space)
    for i = 0, stackSize do
      pos.x = offset.x + i * (diff.x * 2.0 + space)

      local e = createEntity()
      e:emplace(Transform(startPosition + pos))
      e:emplace(ColliderComponent(prefab.collider))
      e:emplace(RigidBody(RigidBodySettings({
        motionType = MotionType.Dynamic,
        mass = 10,
        friction = 0.8,
      })))
      e:emplace(MeshInstance(prefab.mesh))
    end
  end

  offset.x = offset.x + diff.x
  offset.z = offset.z + diff.z
  pos.y = pos.y + (diff.y * 2.0 + space)
  stackSize = stackSize - 1
end
