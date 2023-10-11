#include "WaveDecoder.hpp"
#include <array>
#include <cassert>

namespace {

// http://soundfile.sapp.org/doc/WaveFormat/
// https://replaygain.hydrogenaud.io/wav_format.txt

using ChunkID = std::array<char, 4>;
bool operator==(const ChunkID &id, const std::string_view tag) {
  return std::string_view{id} == tag;
}

struct WaveFileHeader {
  ChunkID riff;
  uint32_t size;
  ChunkID wave;

  explicit operator bool() const { return riff == "RIFF" && wave == "WAVE"; }
};
struct WaveRiffChunk {
  ChunkID id;
  uint32_t size;
};

struct GUID {
  uint32_t data1;
  uint16_t data2;
  uint16_t data3;
  uint8_t data4[8];
};

struct WaveFmtChunk {
  enum class FormatTag : uint16_t {
    PCM = 0x0001,       // Microsoft Pulse Code Modulation format.
    IBM_MULAW = 0x0101, // IBM mu-law format.
    IBM_ALAW = 0x0102,  // IBM a-law format.
    IBM_ADPCM = 0x0103, // Adaptive Differential Pulse Code Modulation format.

    Extensible = 0xFFFE,
  };
  // PCM = 1 (i.e. Linear quantization)
  // Values other than 1 indicate some form of compression.
  FormatTag formatTag;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;   // == sampleRate * numChannels * bitsPerSample / 8
  uint16_t blockAlign; // == numChannels * bitsPerSample / 8
  uint16_t bitsPerSample;

  // if PCM, then doesn't exist.
  uint16_t extraParamSize;
  uint16_t extraParams;
  uint32_t channelMask;
  GUID subFormat{};
};

} // namespace

//
// WaveDecoder class:
//

namespace audio {

WaveDecoder::WaveDecoder(std::unique_ptr<os::DataStream> &&dataStream)
    : m_dataStream{std::move(dataStream)} {
  if (isOpen()) {
    _readInfo();
    rewind();
  }
}

bool WaveDecoder::isOpen() const {
  return m_dataStream && m_dataStream->isOpen();
}
const ClipInfo &WaveDecoder::getInfo() const { return m_info; }

fsec WaveDecoder::tell() {
  return fsec{isOpen() ? _getSampleOffset() / m_info.byteRate() : 0.0f};
}
void WaveDecoder::seek(const fsec offset) {
  if (isOpen()) _setSampleOffset(offset.count() * m_info.byteRate());
}

std::size_t WaveDecoder::read(std::byte *buffer, const std::size_t length) {
  assert(buffer && length > 0);
  if (!isOpen()) return 0;

  const auto available = m_info.dataSize() - _getSampleOffset();
  return available > 0 ? m_dataStream->read(buffer, std::min(length, available))
                       : 0;
}

//
// (private):
//

void WaveDecoder::_readInfo() {
  WaveFileHeader header{};
  m_dataStream->read(&header, sizeof(WaveFileHeader));
  if (!header) throw std::runtime_error{"Not a WAVE file."};

  WaveRiffChunk riff{};
  WaveFmtChunk fmt{};
  while (m_dataStream->read(&riff, sizeof(WaveRiffChunk)) ==
         sizeof(WaveRiffChunk)) {
    if (riff.id == "fmt ") {
      if (riff.size <= sizeof(WaveFmtChunk)) {
        m_dataStream->read(&fmt, riff.size);
        if (fmt.formatTag != WaveFmtChunk::FormatTag::PCM) {
          throw std::runtime_error{
            std::format("Unsupported format tag: {:#x}.",
                        std::to_underlying(fmt.formatTag))};
        }
        m_info = {
          .numChannels = static_cast<NumChannels>(fmt.numChannels),
          .bitsPerSample = uint8_t(fmt.bitsPerSample),
          .sampleRate = fmt.sampleRate,
        };
        continue;
      } else {
        throw std::runtime_error{"Malformed 'fmt ' chunk."};
      }
    } else if (riff.id == "data") {
      m_info.numSamples = riff.size / fmt.blockAlign;
      m_dataOffset = m_dataStream->tell();
    }
    // Jump to next chunk.
    m_dataStream->seek(riff.size, os::DataStream::Origin::Current);
    if (riff.size & 1) m_dataStream->seek(1, os::DataStream::Origin::Current);
  }
}

void WaveDecoder::_setSampleOffset(const std::size_t offset) {
  m_dataStream->seek(m_dataOffset + offset, os::DataStream::Origin::Beginning);
}
std::size_t WaveDecoder::_getSampleOffset() const {
  return m_dataStream->tell() - m_dataOffset;
}

} // namespace audio
