---@meta

---@class mat4
---@overload fun(): mat4
---@overload fun(s: number): mat4
---@overload fun(v0: vec4, v1: vec4, v2: vec4, v3: vec4): mat4
---@overload fun(x0: number, y0: number, z0: number, w0: number, x1: number, y1: number, z1: number, w1: number, x2: number, y2: number, z2: number, w2: number, x3: number, y3: number, z3: number, w3: number): mat4
---@overload fun(m: mat4): mat4
---@operator add(number): mat4
---@operator add(mat4): mat4
---@operator sub(number): mat4
---@operator sub(mat4): mat4
---@operator mul(number): mat4
---@operator mul(vec4): mat4
---@operator mul(mat4): mat4
---@operator div(number): mat4
---@operator div(vec4): mat4
---@operator div(mat4): mat4
math.mat4 = {}
