---@meta

--- COMMON:

---@param x number
---@return number
---@overload fun(x: vec2): vec2
---@overload fun(x: vec3): vec3
---@overload fun(x: vec4): vec4
function math.abs(x) end

---@param x number
---@return number
---@overload fun(x: vec2): vec2
---@overload fun(x: vec3): vec3
---@overload fun(x: vec4): vec4
function math.floor(x) end

---@param x number
---@return number
---@overload fun(x: vec2): vec2
---@overload fun(x: vec3): vec3
---@overload fun(x: vec4): vec4
function math.ceil(x) end

---@param x number
---@param y number
---@return number
---@overload fun(x: vec2, y: vec2): vec2
---@overload fun(x: vec3, y: vec3): vec3
function math.min(x, y) end

---@param x number
---@param y number
---@return number
---@overload fun(x: vec2, y: vec2): vec2
---@overload fun(x: vec3, y: vec3): vec3
function math.max(x, y) end

---@param x number
---@param minVal number
---@param maxVal number
---@return number
---@overload fun(x: vec2, minVal: vec2, maxVal: vec2): vec2
---@overload fun(x: vec3, minVal: vec3, maxVal: vec3): vec3
function math.clamp(x, minVal, maxVal) end

---@param x number
---@param y number
---@param a number
---@return number
---@overload fun(x: vec2, y: vec2, a: vec2): vec2
---@overload fun(x: vec3, y: vec3, a: vec3): vec3
function math.mix(x, y, a) end

--- TRIGONOMETRIC:

---@param x number|vec2|vec3|vec4
---@return number
function math.degrees(x) end

---@param x number|vec2|vec3|vec4
---@return number
function math.radians(x) end

---@param angle number
---@return number
function math.sin(angle) end

---@param angle number
---@return number
function math.cos(angle) end

---@param angle number
---@return number
function math.tan(angle) end

--- EXPONENTIAL:

---@param v number
---@return number
---@overload fun(v: vec2): vec2
---@overload fun(v: vec3): vec3
function math.exp(v) end

---@param v number
---@return number
---@overload fun(v: vec2): vec2
---@overload fun(v: vec3): vec3
function math.exp2(v) end

---@param v number
---@return number
---@overload fun(v: vec2): vec2
---@overload fun(v: vec3): vec3
function math.inversesqrt(v) end

---@param v number
---@return number
---@overload fun(v: vec2): vec2
---@overload fun(v: vec3): vec3
function math.log(v) end

---@param v number
---@return number
---@overload fun(v: vec2): vec2
---@overload fun(v: vec3): vec3
function math.log2(v) end

---@param base number
---@param exponent number
---@return number
---@overload fun(base: vec2, exponent: vec2): vec2
---@overload fun(base: vec3, exponent: vec3): vec3
function math.pow(base, exponent) end

---@param v number
---@return number
---@overload fun(v: vec2): vec2
---@overload fun(v: vec3): vec3
function math.sqrt(v) end

--- GEOMETRIC:

---@param x vec2
---@param y vec2
---@return number
---@overload fun(x: vec3, y: vec3): number
---@overload fun(x: vec4, y: vec4): number
---@overload fun(x: quat, y: quat): number
function math.dot(x, y) end

---@param x vec3
---@param y vec3
---@return vec3
---@overload fun(x: quat, y: quat): quat
function math.cross(x, y) end

---@param p0 vec2
---@param p1 vec2
---@return number
---@overload fun(p0: vec3, y: vec3): number
function math.distance(p0, p1) end

---@param v vec2|vec3
---@return number
function math.length(v) end

---@param I vec2
---@param N vec2
---@return vec2
---@overload fun(I: vec3, N: vec3): vec3
function math.reflect(I, N) end

---@param I vec2
---@param N vec2
---@param eta number
---@return vec2
---@overload fun(I: vec3, N: vec3, eta: number): vec3
function math.refract(I, N, eta) end

---@param v vec2
---@return vec2
---@overload fun(v: vec3): vec3
---@overload fun(v: vec4): vec4
---@overload fun(v: quat): quat
function math.normalize(v) end

--- MATRIX:

---@param m mat4
---@return mat4
function math.inverse(m) end

---@param m mat4
---@return mat4
function math.transpose(m) end

--- NOISE:

---@param p vec2
---@return number
---@overload fun(x: number, y: number): number
function math.noise(p) end
