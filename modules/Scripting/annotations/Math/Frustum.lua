--- @meta

--- @class Frustum
math.Frustum = {}

--- @param p vec3
--- @return boolean
function math.Frustum:testPoint(p) end

--- @param aabb AABB
--- @return boolean
function math.Frustum:testAABB(aabb) end

--- @param sphere Sphere
--- @return boolean
function math.Frustum:testSphere(sphere) end

--- @param cone Cone
--- @return boolean
function math.Frustum:testCone(cone) end
