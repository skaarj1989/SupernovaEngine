---@meta

---@class Cone
---@field T vec3 # Cone tip.
---@field h number # Height of the cone.
---@field d vec3 # Direction of the cone.
---@field r number # Bottom radius of the cone.
---@overload fun(T: vec3, h: number, d: vec3, r: number): Cone
math.Cone = {}
