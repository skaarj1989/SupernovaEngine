---@meta

---@class InputSystem
InputSystem = {}

---@param p ivec2
function InputSystem:setMousePosition(p) end

---@return ivec2
function InputSystem:getMousePosition() end

---@return ivec2
function InputSystem:getMouseDelta() end

---@param b boolean
function InputSystem:showCursor(b) end

---@return boolean
function InputSystem:isCursorVisible() end

---@param button MouseButton
---@return boolean
function InputSystem:isMouseDown(button) end

---@param button MouseButton
---@return boolean
function InputSystem:isMouseUp(button) end

---@param key KeyCode
---@return boolean
function InputSystem:isKeyDown(key) end

---@param key KeyCode
---@return boolean
function InputSystem:isKeyUp(key) end
