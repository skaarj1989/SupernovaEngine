---@meta

---@class PlaybackController : ComponentBase
PlaybackController = {}

---@param ratio number
---@return self
function PlaybackController:setTimeRatio(ratio) end

---@param f number
---@return self
function PlaybackController:setSpeed(f) end

---@param b boolean
---@return self
function PlaybackController:setLoop(b) end

---@return number
function PlaybackController:getTimeRatio() end

---@return number
function PlaybackController:getPreviousTimeRatio() end

---@return number
function PlaybackController:getPlaybackSpeed() end

---@return boolean
function PlaybackController:isPlaying() end

---@return boolean
function PlaybackController:isLooped() end

---@return self
function PlaybackController:play() end

---@return self
function PlaybackController:pause() end

---@return self
function PlaybackController:resume() end

---@return self
function PlaybackController:stop() end

---@return self
function PlaybackController:reset() end
