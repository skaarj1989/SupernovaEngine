---@class Template: ScriptNode
local node = {}

function node:init() end

---@param evt InputEvent
function node:input(evt) end

---@param dt number
function node:update(dt) end

---@param dt number
function node:physicsStep(dt) end

function node:destroy() end

return node
