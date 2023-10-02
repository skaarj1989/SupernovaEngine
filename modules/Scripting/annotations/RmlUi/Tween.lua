---@meta

---@enum ui.TweenType
ui.TweenType = {
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

---@enum ui.TweenDirection
ui.TweenDirection = {
  In = 1, Out = 2, InOut = 3,
}

---@class ui.Tween
---@overload fun(type?: ui.TweenType, direction?: ui.TweenDirection): ui.Tween
---@overload fun(typeIn: ui.TweenType, typeOut: ui.TweenType): ui.Tween
ui.Tween = {}

---Reverse direction of the tweening function.
function ui.Tween:reverse() end
