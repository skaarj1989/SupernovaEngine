---@meta

---@class AudioWorld
AudioWorld = {}

---@param settings audio.Device.Settings
---@return self
function AudioWorld:setSettings(settings) end

---@return audio.Device.Settings
function AudioWorld:getSettings() end

---@return audio.Device
function AudioWorld:getDevice() end

---@param path string
---@return audio.Decoder
function createStream(path) end
