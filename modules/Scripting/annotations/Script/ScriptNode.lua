---@meta

---@class ScriptNode
---@field id integer
---@field entity Entity
ScriptNode = {}

function ScriptNode:init() end

---@param evt InputEvent
function ScriptNode:input(evt) end

---@param dt number
function ScriptNode:update(dt) end

---@param dt number
function ScriptNode:physicsStep(dt) end

function ScriptNode:destroy() end
