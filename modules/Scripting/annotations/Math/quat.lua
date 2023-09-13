--- @meta

--- @class quat
--- @field x number
--- @field y number
--- @field z number
--- @field w number
--- @overload fun(): quat
--- @overload fun(w: number, x: number, y: number, z: number): quat
--- @overload fun(eulerAngles: vec3): quat
--- @overload fun(q: quat): quat
--- @operator add(quat): quat
--- @operator sub(quat): quat
--- @operator mul(number): quat
--- @operator mul(vec3): vec3
--- @operator mul(vec4): vec4
--- @operator mul(quat): quat
--- @operator div(number): quat
math.quat = {}

--- @return vec3
--- @param q quat
function math.eulerAngles(q) end

--- @return quat
--- @param angle number
--- @param v vec3
function math.angleAxis(angle, v) end

--- @return quat
--- @param direction vec3
--- @param up vec3
function math.lookAt(direction, up) end
