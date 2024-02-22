local prefab <const> = {
  mesh = loadMesh("BasicShapes:Cube"),
  collider = loadCollider("Colliders/Box")
}

-- Config:

local startPosition = math.vec3(0)

local boxSize <const> = 0.5
local stackSize = 12

-- Create a wall:

local diff <const> = math.vec3(boxSize * 1.0)
local offset = -stackSize * (diff.z * 2.0) * 0.5
local pos = math.vec3(0.0, diff.y, 0.0)

while stackSize >= 0 do
  for i = 0, stackSize do
    pos.z = offset + i * (diff.z * 2.0)

    local e = createEntity()
    e:emplace(Transform(startPosition + pos))
    e:emplace(ColliderComponent(prefab.collider))
    e:emplace(RigidBody(RigidBody.Settings({
      motionType = MotionType.Dynamic,
      mass = 10,
      friction = 0.8,
    })))
    e:emplace(MeshInstance(prefab.mesh))
  end

  offset = offset + diff.z
  pos.y = pos.y + (diff.y * 2.0)
  stackSize = stackSize - 1
end
