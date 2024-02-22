---@meta

---@class ui.Tween
---@overload fun(type?: ui.Tween.Type, direction?: ui.Tween.Direction): ui.Tween
---@overload fun(typeIn: ui.Tween.Type, typeOut: ui.Tween.Type): ui.Tween
ui.Tween = {}

---@enum ui.Tween.Type
ui.Tween.Type = {
  None = 0,
  Back = 1,
  Bounce = 2,
  Circular = 3,
  Cubic = 4,
  Elastic = 5,
  Exponential = 6,
  Linear = 7,
  Quadratic = 8,
  Quartic = 9,
  Quintic = 10,
  Sine = 11,
  Callback = 12,
  Count = 13,
}

---@enum ui.Tween.Direction
ui.Tween.Direction = {
  In = 1, Out = 2, InOut = 3,
}

---Reverse direction of the tweening function.
function ui.Tween:reverse() end
