---@meta

---@class SoundSourceComponent : ComponentBase
SoundSourceComponent = {}

---@param clip AudioClipResource
---@return self
function SoundSourceComponent:setClip(clip) end

---@param settings SoundSettings
---@return self
function SoundSourceComponent:setSettings(settings) end

---@param s number
---@return self
function SoundSourceComponent:setPitch(s) end

---@param s number
---@return self
function SoundSourceComponent:setGain(s) end

---@param s number
---@return self
function SoundSourceComponent:setMaxDistance(s) end

---@param s number
---@return self
function SoundSourceComponent:setRollOffFactor(s) end

---@param s number
---@return self
function SoundSourceComponent:setReferenceDistance(s) end

---@param s number
---@return self
function SoundSourceComponent:setMinGain(s) end

---@param s number
---@return self
function SoundSourceComponent:setMaxGain(s) end

---@param b boolean
---@return self
function SoundSourceComponent:setDirectional(b) end

---@param b boolean
---@return self
function SoundSourceComponent:setLooping(b) end

---@param v vec3
---@return self
function SoundSourceComponent:setVelocity(v) end

---@return AudioClipResource
function SoundSourceComponent:getClip() end

---@return SoundSettings
function SoundSourceComponent:getSettings() end

---@return vec3
function SoundSourceComponent:getVelocity() end

---@return self
function SoundSourceComponent:play() end

---@return self
function SoundSourceComponent:pause() end

---@return self
function SoundSourceComponent:stop() end
