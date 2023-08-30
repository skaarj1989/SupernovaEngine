--- @meta

--- @class vec4
--- @field x number
--- @field y number
--- @field z number
--- @field w number
--- @field r number
--- @field g number
--- @field b number
--- @field a number
--- @overload fun(): vec4
--- @overload fun(v: number|vec4|ivec4|uvec4): vec4
--- @overload fun(v0: vec2|ivec2|uvec2, v1: vec2|ivec2|uvec2): vec4
--- @overload fun(v0: vec3|ivec3|uvec3, v1: number): vec4
--- @overload fun(v0: vec2|ivec2|uvec2, v1: number, v2: number): vec4
--- @overload fun(x: number, y: number, z: number, w: number): vec4
--- @operator add(number): vec4
--- @operator add(vec4): vec4
--- @operator sub(number): vec4
--- @operator sub(vec4): vec4
--- @operator mul(number): vec3
--- @operator mul(vec4): vec4
--- @operator mul(quat): vec4
--- @operator mul(mat4): vec4
--- @operator div(number): vec4
--- @operator div(vec4): vec4
--- @operator div(mat4): vec4
math.vec4 = {}

--- @class ivec4
--- @field x integer
--- @field y integer
--- @field z integer
--- @field w integer
--- @overload fun(): ivec4
--- @overload fun(v: number|vec4|ivec4|uvec4): ivec4
--- @overload fun(v0: vec2|ivec2|uvec2, v1: vec2|ivec2|uvec2): ivec4
--- @overload fun(v0: vec3|ivec3|uvec3, v1: number): ivec4
--- @overload fun(v0: vec2|ivec2|uvec2, v1: number, v2: number): ivec4
--- @overload fun(x: number, y: number, z: number, w: number): ivec4
--- @operator add(number): ivec4
--- @operator add(ivec4): ivec4
--- @operator sub(number): ivec4
--- @operator sub(ivec4): ivec4
--- @operator mul(number): ivec4
--- @operator mul(ivec4): ivec4
--- @operator div(number): ivec4
--- @operator div(ivec4): ivec4
math.ivec4 = {}

--- @class uvec4
--- @field x integer
--- @field y integer
--- @field z integer
--- @field w integer
--- @overload fun(): uvec4
--- @overload fun(v: number|vec4|ivec4|uvec4): uvec4
--- @overload fun(v0: vec2|ivec2|uvec2, v1: vec2|ivec2|uvec2): uvec4
--- @overload fun(v0: vec3|ivec3|uvec3, v1: number): uvec4
--- @overload fun(v0: vec2|ivec2|uvec2, v1: number, v2: number): uvec4
--- @overload fun(x: number, y: number, z: number, w: number): uvec4
--- @operator add(number): uvec4
--- @operator add(uvec4): uvec4
--- @operator sub(number): uvec4
--- @operator sub(uvec4): uvec4
--- @operator mul(number): uvec4
--- @operator mul(uvec4): uvec4
--- @operator div(number): uvec4
--- @operator div(uvec4): uvec4
math.uvec4 = {}

--- @class bvec4
--- @field x boolean
--- @field y boolean
--- @field z boolean
--- @field w boolean
math.bvec4 = {}
