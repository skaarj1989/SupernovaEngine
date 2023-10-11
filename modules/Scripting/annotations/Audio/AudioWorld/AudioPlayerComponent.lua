---@meta

---@class AudioPlayerComponent : ComponentBase
AudioPlayerComponent = {}

---@param stream audio.Decoder
---@return self
function AudioPlayerComponent:setStream(stream) end

---@param settings SoundSettings
---@return self
function AudioPlayerComponent:setSettings(settings) end

---@param s number
---@return self
function AudioPlayerComponent:setPitch(s) end

---@param s number
---@return self
function AudioPlayerComponent:setGain(s) end

---@param s number
---@return self
function AudioPlayerComponent:setMaxDistance(s) end

---@param s number
---@return self
function AudioPlayerComponent:setRollOffFactor(s) end

---@param s number
---@return self
function AudioPlayerComponent:setReferenceDistance(s) end

---@param s number
---@return self
function AudioPlayerComponent:setMinGain(s) end

---@param s number
---@return self
function AudioPlayerComponent:setMaxGain(s) end

---@param b boolean
---@return self
function AudioPlayerComponent:setDirectional(b) end

---@param b boolean
---@return self
function AudioPlayerComponent:setLooping(b) end

---@param v vec3
---@return self
function AudioPlayerComponent:setVelocity(v) end

---@return SoundSettings
function AudioPlayerComponent:getSettings() end

---@return vec3
function AudioPlayerComponent:getVelocity() end

---@return self
function AudioPlayerComponent:play() end

---@return self
function AudioPlayerComponent:pause() end

---@return self
function AudioPlayerComponent:stop() end
