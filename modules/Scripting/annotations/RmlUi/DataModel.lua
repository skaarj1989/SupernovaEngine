---@meta

---@class ui.DataModelHandle
ui.DataModelHandle = {}

---@param variableName string
---@return boolean
function ui.DataModelHandle:isVariableDirty(variableName) end

---@param variableName string
function ui.DataModelHandle:dirtyVariable(variableName) end

function ui.DataModelHandle:dirtyAllVariables() end

---@class ui.ScriptDataModel
ui.ScriptDataModel = {}

---@return ui.DataModelHandle
function ui.ScriptDataModel:getModelHandle() end
