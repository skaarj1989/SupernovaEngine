---@meta

---@class Emitter
Emitter = {}

---@generic T: EventBase
---@param type T
---@param listener fun(evt: T, sender: Emitter)
function Emitter:connect(type, listener) end

---@generic T: EventBase
---@param type T
function Emitter:disconnect(type) end
