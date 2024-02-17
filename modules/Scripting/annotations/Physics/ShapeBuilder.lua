---@meta

---@class UncookedShape
UncookedShape = {}

---@return boolean
function UncookedShape:isValid() end

---@class ShapeBuilder
---@overload fun(): ShapeBuilder
ShapeBuilder = {}

---@class ShapeBuilder.Transform
---@field position vec3
---@field orientation quat
ShapeBuilder.Transform = {}

---@return boolean
function ShapeBuilder.Transform:isIdentity() end

---@param radius number
---@return UncookedShape
function ShapeBuilder.makeSphere(radius) end

---@param halfExtent vec3
---@return UncookedShape
function ShapeBuilder.makeBox(halfExtent) end

---@param halfHeight number
---@param radius number
---@return UncookedShape
function ShapeBuilder.makeCapsule(halfHeight, radius) end

---@param points vec3[]
---@return UncookedShape
function ShapeBuilder.makeConvexHull(points) end

---@param uncooked UncookedShape
---@param xf? ShapeBuilder.Transform
---@return self
function ShapeBuilder:set(uncooked, xf) end

---@return self
function ShapeBuilder:reset() end

---@param uncooked UncookedShape
---@param xf? ShapeBuilder.Transform
---@return self
function ShapeBuilder:add(uncooked, xf) end

---@param v vec3
---@return self
function ShapeBuilder:scale(v) end

---@param position vec3
---@param orientation quat
---@return self
function ShapeBuilder:rotateTranslate(position, orientation) end

---@return integer
function ShapeBuilder:size() end

---@return UncookedShape
function ShapeBuilder:build() end
