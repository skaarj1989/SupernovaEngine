---@meta

---@class vec3
---@field x number
---@field y number
---@field z number
---@field r number
---@field g number
---@field b number
---@overload fun(): vec3
---@overload fun(v: number|vec3|vec4|ivec3|ivec4|uvec3|uvec4): vec3
---@overload fun(x: number, y: number, z: number): vec3
---@operator add(number): vec3
---@operator add(vec3): vec3
---@operator sub(number): vec3
---@operator sub(vec3): vec3
---@operator mul(number): vec3
---@operator mul(vec3): vec3
---@operator mul(quat): vec3
---@operator div(number): vec3
---@operator div(vec3): vec3
math.vec3 = {}

---@return vec3[]
function math.vec3.makeArray() end

---@class ivec3
---@field x integer
---@field y integer
---@field z integer
---@overload fun(): ivec3
---@overload fun(v: number|vec3|vec4|ivec3|ivec4|uvec3|uvec4): ivec3
---@overload fun(x: number, y: number, z: number): ivec3
---@operator add(number): ivec3
---@operator add(ivec3): ivec3
---@operator sub(number): ivec3
---@operator sub(ivec3): ivec3
---@operator mul(number): ivec3
---@operator mul(ivec3): ivec3
---@operator div(number): ivec3
---@operator div(ivec3): ivec3
math.ivec3 = {}

---@return ivec3[]
function math.ivec3.makeArray() end

---@class uvec3
---@field x integer
---@field y integer
---@field z integer
---@overload fun(): uvec3
---@overload fun(v: number|vec3|vec4|ivec3|ivec4|uvec3|uvec4): uvec3
---@overload fun(x: number, y: number, z: number): uvec3
---@operator add(number): uvec3
---@operator add(uvec3): uvec3
---@operator sub(number): uvec3
---@operator sub(uvec3): uvec3
---@operator mul(number): uvec3
---@operator mul(uvec3): uvec3
---@operator div(number): uvec3
---@operator div(uvec3): uvec3
math.uvec3 = {}

---@return uvec3[]
function math.uvec3.makeArray() end

---@class bvec3
---@field x boolean
---@field y boolean
---@field z boolean
---@overload fun(): bvec3
---@overload fun(v: bvec3): bvec3
---@overload fun(x: boolean, y: boolean, z: boolean): bvec3
math.bvec3 = {}

---@return bvec3[]
function math.bvec3.makeArray() end
