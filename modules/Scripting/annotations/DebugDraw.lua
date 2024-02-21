---@meta

---@class DebugDraw
DebugDraw = {}

---@param position vec3
---@param size number
---@param color? vec3
---@return self
function DebugDraw:addPoint(position, size, color) end

---@param origin vec3
---@param _end vec3
---@param color? vec3
---@return self
function DebugDraw:addLine(origin, _end, color) end

---@param radius number
---@param color? vec3
---@param m? mat4
---@return self
function DebugDraw:addCircle(radius, color, m) end

---@param radius number
---@param color? vec3
---@param m? mat4
---@return self
function DebugDraw:addSphere(radius, color, m) end

---@param aabb AABB
---@param color? vec3
---@return self
function DebugDraw:addAABB(aabb, color) end

---@param inversedViewProj mat4
---@param color? vec3
---@return self
function DebugDraw:addFrustum(inversedViewProj, color) end

---@return boolean
function DebugDraw:empty() end
