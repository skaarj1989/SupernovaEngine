---@meta

---@class PerspectiveCamera
---@overload fun(): PerspectiveCamera
PerspectiveCamera = {}

---@param fov number
---@param aspectRatio number
---@param clippingPlanes ClippingPlanes
---@return self
function PerspectiveCamera:setPerspective(fov, aspectRatio, clippingPlanes) end

---@param s number
---@return self
function PerspectiveCamera:setFov(s) end

---@param s number
---@return self
function PerspectiveCamera:setAspectRatio(s) end

---@param v ClippingPlanes
---@return self
function PerspectiveCamera:setClippingPlanes(v) end

---@param v vec3
---@return self
function PerspectiveCamera:setPosition(v) end

---@param q quat
---@return self
function PerspectiveCamera:setOrientation(q) end

---@param xf Transform
---@return self
function PerspectiveCamera:fromTransform(xf) end

---@return number
function PerspectiveCamera:getFov() end

---@return number
function PerspectiveCamera:getAspectRatio() end

---@return ClippingPlanes
function PerspectiveCamera:getClippingPlanes() end

---@return vec3
function PerspectiveCamera:getPosition() end

---@return quat
function PerspectiveCamera:getOrientation() end

---@return number
function PerspectiveCamera:getPitch() end

---@return number
function PerspectiveCamera:getYaw() end

---@return number
function PerspectiveCamera:getRoll() end

---@return vec3
function PerspectiveCamera:getRight() end

---@return vec3
function PerspectiveCamera:getUp() end

---@return vec3
function PerspectiveCamera:getForward() end

---@return mat4
function PerspectiveCamera:getView() end

---@return mat4
function PerspectiveCamera:getProjection() end

---@return mat4
function PerspectiveCamera:getViewProjection() end

---@return Frustum
function PerspectiveCamera:getFrustum() end

---@class ClippingPlanes
---@field zNear number
---@field zFar number
---@overload fun(zNear: number, zFar: number): ClippingPlanes
ClippingPlanes = {}
