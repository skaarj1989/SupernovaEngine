--- @meta

--- @class vec3
--- @field x number
--- @field y number
--- @field z number
--- @field r number
--- @field g number
--- @field b number
--- @overload fun(): vec3
--- @overload fun(v: number|vec3|ivec3|uvec3): vec3
--- @overload fun(x: number, y: number, z: number): vec3
--- @operator add(number): vec3
--- @operator add(vec3): vec3
--- @operator sub(number): vec3
--- @operator sub(vec3): vec3
--- @operator mul(number): vec3
--- @operator mul(vec3): vec3
--- @operator mul(quat): vec3
--- @operator div(number): vec3
--- @operator div(vec3): vec3
math.vec3 = {}

--- @class ivec3
--- @field x integer
--- @field y integer
--- @field z integer
--- @operator add(number): ivec3
--- @operator add(ivec3): ivec3
--- @operator sub(number): ivec3
--- @operator sub(ivec3): ivec3
--- @operator mul(number): ivec3
--- @operator mul(ivec3): ivec3
--- @operator div(number): ivec3
--- @operator div(ivec3): ivec3
math.ivec3 = {}

--- @class uvec3
--- @field x integer
--- @field y integer
--- @field z integer
--- @operator add(number): uvec3
--- @operator add(uvec3): uvec3
--- @operator sub(number): uvec3
--- @operator sub(uvec3): uvec3
--- @operator mul(number): uvec3
--- @operator mul(uvec3): uvec3
--- @operator div(number): uvec3
--- @operator div(uvec3): uvec3
math.uvec3 = {}

--- @class bvec3
--- @field x boolean
--- @field y boolean
--- @field z boolean
math.bvec3 = {}
