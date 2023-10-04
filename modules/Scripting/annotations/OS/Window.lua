---@meta

---@class Window
Window = {}

---@param v ivec2
---@return self
function Window:setPosition(v) end

---@param v ivec2
---@return self
function Window:setSize(v) end

---@param a number
---@return self
function Window:setAlpha(a) end

---@param s string
---@return self
function Window:setCaption(s) end

---@return ivec2
function Window:getPosition() end

---@return ivec2
function Window:getSize() end

---@return uvec2
function Window:getClientSize() end

---@return string
function Window:getCaption() end

---@return boolean
function Window:isMinimized() end

---@return boolean
function Window:hasFocus() end

---@return self
function Window:show() end

---@return self
function Window:hide() end

---@return self
function Window:minimize() end

---@return self
function Window:maximize() end

---@return self
function Window:focus() end

function Window:close() end

---@param w Window
---@return number
function getAspectRatio(w) end

---@param w Window
function center(w) end

---@param w Window
---@return uvec2
function getCenter(w) end

---@type Window
GameWindow = {}
