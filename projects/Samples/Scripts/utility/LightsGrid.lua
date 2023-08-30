local startPosition <const> = math.vec3(0)
local dimensions <const> = {
  width = 10,
  height = 10,
  depth = 10
}
local step <const> = math.vec3(3, 3, 3)

math.randomseed(os.time())

local getRandomColor <const> = function()
  return math.vec3(
    math.random(0.0, 1.0),
    math.random(0.0, 1.0),
    math.random(0.0, 1.0)
  )
end

for x = 0, dimensions.width - 1 do
  for y = 0, dimensions.height - 1 do
    for z = 0, dimensions.depth - 1 do
      local e = createEntity()
      e:emplace(Transform(startPosition + math.vec3(x, y, z) * step))
      e:emplace(Light({
        type = LightType.Point,
        color = getRandomColor(),
        intensity = 100.0,
        range = 4.0,
        castsShadow = false,
        debugVolume = false
      }))
    end
  end
end
