---@meta

---@class Window
Window = {}

---@enum Window.Area
Window.Area = {
  Client = 0,
  Absolute = 1
}

---@param v ivec2
---@return self
function Window:setPosition(v) end

---@param v ivec2
---@return self
function Window:setExtent(v) end

---@param a number
---@return self
function Window:setAlpha(a) end

---@param s string
---@return self
function Window:setCaption(s) end

---@param area? Window.Area
---@return ivec2
function Window:getPosition(area) end

---@param area? Window.Area
---@return ivec2
function Window:getExtent(area) end

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
