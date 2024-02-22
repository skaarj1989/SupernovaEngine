local prefab <const> = {
  mesh = loadMesh("BasicShapes:Cube"),
  collider = loadCollider("Colliders/Box")
}

local startPosition <const> = math.vec3(0.0, 2.5, 0.0)
local dimensions <const> = {
  width = 3,
  height = 6, -- Number of rows (y axis).
  depth = 3
}
local spacing <const> = 1.0

for row = 0, dimensions.height - 1 do
  for col = 0, dimensions.width - 1 do
    for z = 0, dimensions.depth - 1 do
      local localPosition = math.vec3(
        (col - (dimensions.height / 2)) * spacing,
        row * spacing + spacing,
        (z - (dimensions.depth / 2)) * spacing
      )

      local e = createEntity()
      e:emplace(Transform(startPosition + localPosition))
      e:emplace(ColliderComponent(prefab.collider))
      e:emplace(RigidBody(RigidBody.Settings({
        motionType = MotionType.Dynamic,
        mass = 10,
        friction = 0.8,
      })))
      e:emplace(MeshInstance(prefab.mesh))
    end
  end
end
