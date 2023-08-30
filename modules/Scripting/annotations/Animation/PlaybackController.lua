--- @meta

--- @class PlaybackController : ComponentBase
PlaybackController = {}

--- @param ratio number
--- @return PlaybackController self
function PlaybackController:setTimeRatio(ratio) end

--- @param f number
--- @return PlaybackController self
function PlaybackController:setSpeed(f) end

--- @param b boolean
--- @return PlaybackController self
function PlaybackController:setLoop(b) end

--- @return number
function PlaybackController:getTimeRatio() end

--- @return number
function PlaybackController:getPreviousTimeRatio() end

--- @return number
function PlaybackController:getPlaybackSpeed() end

--- @return boolean
function PlaybackController:isPlaying() end

--- @return boolean
function PlaybackController:isLooped() end

--- @return PlaybackController self
function PlaybackController:play() end

--- @return PlaybackController self
function PlaybackController:pause() end

--- @return PlaybackController self
function PlaybackController:resume() end

--- @return PlaybackController self
function PlaybackController:stop() end

--- @return PlaybackController self
function PlaybackController:reset() end
