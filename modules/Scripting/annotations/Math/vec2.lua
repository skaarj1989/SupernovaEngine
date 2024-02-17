---@meta

---@class vec2
---@field x number
---@field y number
---@overload fun(): vec2
---@overload fun(v: number|vec2|vec3|vec4|ivec2|ivec3|ivec4|uvec2|uvec3|uvec4): vec2
---@overload fun(x: number, y: number): vec2
---@operator add(number): vec2
---@operator add(vec2): vec2
---@operator sub(number): vec2
---@operator sub(vec2): vec2
---@operator mul(number): vec2
---@operator mul(vec2): vec2
---@operator div(number): vec2
---@operator div(vec2): vec2
math.vec2 = {}

---@return vec2[]
function math.vec2.makeArray() end

---@class ivec2
---@field x integer
---@field y integer
---@overload fun(): ivec2
---@overload fun(v: number|vec2|vec3|vec4|ivec2|ivec3|ivec4|uvec2|uvec3|uvec4): ivec2
---@overload fun(x: number, y: number): ivec2
---@operator add(number): ivec2
---@operator add(ivec2): ivec2
---@operator sub(number): ivec2
---@operator sub(ivec2): ivec2
---@operator mul(number): ivec2
---@operator mul(ivec2): ivec2
---@operator div(number): ivec2
---@operator div(ivec2): ivec2
math.ivec2 = {}

---@return ivec2[]
function math.ivec2.makeArray() end

---@class uvec2
---@field x integer
---@field y integer
---@overload fun(): uvec2
---@overload fun(v: number|vec2|vec3|vec4|ivec2|ivec3|ivec4|uvec2|uvec3|uvec4): uvec2
---@overload fun(x: number, y: number): uvec2
---@operator add(number): uvec2
---@operator add(uvec2): uvec2
---@operator sub(number): uvec2
---@operator sub(uvec2): uvec2
---@operator mul(number): uvec2
---@operator mul(uvec2): uvec2
---@operator div(number): uvec2
---@operator div(uvec2): uvec2
math.uvec2 = {}

---@return uvec2[]
function math.uvec2.makeArray() end

---@class bvec2
---@field x boolean
---@field y boolean
---@overload fun(): bvec2
---@overload fun(v: bvec2): bvec2
---@overload fun(x: boolean, y: boolean): bvec2
math.bvec2 = {}

---@return bvec2[]
function math.bvec2.makeArray() end
