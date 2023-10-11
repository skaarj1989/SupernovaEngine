---@meta

---@class audio.Decoder
audio.Decoder = {}

---@return boolean
function audio.Decoder:isOpen() end

---@return audio.ClipInfo
function audio.Decoder:getInfo() end

---@enum audio.NumChannels
audio.NumChannels = {
  Invalid = 0,
  Mono = 1,
  Stereo = 2,
  Surround1D = 3,
  QuadSurround = 4,
  Surround = 5,
  Surround5_1 = 6,
  Surround6_1 = 7,
  Surround7_1 = 8,
}

---@class audio.ClipInfo
---@field numChannels audio.NumChannels
---@field bitsPerSample integer
---@field sampleRate integer
---@field numSamples integer
audio.ClipInfo = {}

---@return integer
function audio.ClipInfo:blockAlign() end

---@return integer
function audio.ClipInfo:byteRate() end

---@return integer
function audio.ClipInfo:dataSize() end

---@return number
function audio.ClipInfo:duration() end
