---@meta

---@class Transform : ComponentBase
---@field Right vec3
---@field Up vec3
---@field Forward vec3
---@overload fun(): Transform
---@overload fun(position: vec3, orientation: quat, scale: vec3): Transform
---@overload fun(position: vec3): Transform
---@overload fun(m: mat4): Transform
Transform = {}

---@param m mat4
---@return self
function Transform:load(m) end

---@return self
function Transform:loadIdentity() end

---@param v vec3
---@return self
function Transform:setPosition(v) end

---@param q quat
---@return self
function Transform:setOrientation(q) end

---@param eulerAngles vec3 # In radians
---@return self
function Transform:setEulerAngles(eulerAngles) end

---@param v vec3
---@return self
function Transform:setScale(v) end

---@return mat4
function Transform:getModelMatrix() end

---@return mat4
function Transform:getWorldMatrix() end

---@return vec3
function Transform:getLocalPosition() end

---@return quat
function Transform:getLocalOrientation() end

---@return vec3
function Transform:getLocalEulerAngles() end

---@return vec3
function Transform:getLocalScale() end

---@return vec3
function Transform:getPosition() end

---@return quat
function Transform:getOrientation() end

---@return vec3
function Transform:getEulerAngles() end

---@return vec3
function Transform:getScale() end

---@return vec3
function Transform:getRight() end

---@return vec3
function Transform:getUp() end

---@return vec3
function Transform:getForward() end

---@param v vec3
---@return self
function Transform:translate(v) end

---@param q quat
---@return self
function Transform:rotate(q) end

---@param angle number # In radians
---@return self
function Transform:pitch(angle) end

---@param angle number # In radians
---@return self
function Transform:yaw(angle) end

---@param angle number # In radians
---@return self
function Transform:roll(angle) end

---@param v vec4
---@return self
---@overload fun(xf: Transform): Transform
function Transform:lookAt(v) end

---@param v vec3
---@return self
function Transform:scale(v) end
