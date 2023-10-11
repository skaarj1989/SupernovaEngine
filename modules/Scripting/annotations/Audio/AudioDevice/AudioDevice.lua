---@meta

audio = {}

---@enum audio.DistanceModel
audio.DistanceModel = {
  None = 0,
  Inverse = 1,
  InverseClamped = 2,
  Linear = 3,
  LinearClamped = 4,
  Exponent = 5,
  ExponentClamped = 6,
}

---@class audio.Device.Settings
---@field distanceModel audio.DistanceModel
---@field dopplerFactor number
---@field speedOfSound number
audio.Device.Settings = {}

---@class audio.Device
audio.Device = {}

---@param settings audio.Device.Settings
---@return self
function audio.Device:setSettings(settings) end

---@param distanceModel audio.DistanceModel
---@return self
function audio.Device:setDistanceModel(distanceModel) end

---@param s number
---@return self
function audio.Device:setDopplerFactor(s) end

---@param s number
---@return self
function audio.Device:setSpeedOfSound(s) end

---@param s number
---@return self
function audio.Device:setMasterGain(s) end

---@return audio.Device.Settings
function audio.Device:getSettings() end

---@class audio.Buffer
audio.Buffer = {}

---@return audio.ClipInfo
function audio.Buffer:getInfo() end
