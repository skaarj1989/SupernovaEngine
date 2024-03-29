#include "VorbisDecoder.hpp"
#include <cassert>

namespace {

// F - Front
// S - Side
// R - Rear
// LFE - Subwoofer
//
// Vorbis channel order:
//   2 (stereo): L, R
//   3 (1D surround): L, C, R
//   4 (quadrophonic surround): FL, FR, RL, RR
//   5 (surround): FL, C, FR, RL, RR
//   6 (5.1 surround): FL, C, FR, RL, RR, LFE
//   7 (6.1 surround): FL, C, FR, SL, SR, RC, LFE
//   8 (7.1 surround): FL, C, FR, SL, SR, RL, RR, LFE

size_t ogg_read_cb(void *buffer, size_t size, size_t nmemb, void *datasource) {
  auto *dataSource = static_cast<os::DataStream *>(datasource);
  return dataSource ? dataSource->read(buffer, size * nmemb) : -1;
}
int ogg_seek_cb(void *datasource, ogg_int64_t offset, int32_t whence) {
  auto *dataSource = static_cast<os::DataStream *>(datasource);
  return dataSource ? dataSource->seek(
                        offset, static_cast<os::DataStream::Origin>(whence))
                    : -1;
}
int ogg_close_cb(void *datasource) {
  auto *dataSource = static_cast<os::DataStream *>(datasource);
  return dataSource ? dataSource->close() : -1;
}
long ogg_tell_cb(void *datasource) {
  const auto *dataSource = static_cast<os::DataStream *>(datasource);
  return dataSource ? dataSource->tell() : -1;
}

const ov_callbacks g_callbacks{
  ogg_read_cb,
  ogg_seek_cb,
  ogg_close_cb,
  ogg_tell_cb,
};

} // namespace

//
// VorbisDecoder class:
//

namespace audio {

VorbisDecoder::VorbisDecoder(std::unique_ptr<os::DataStream> &&dataStream)
    : m_dataStream{std::move(dataStream)} {
  const auto result =
    ov_open_callbacks(m_dataStream.get(), &m_vf, nullptr, 0, g_callbacks);
  if (result != 0) throw std::runtime_error{"Not a Vorbis file."};

  const auto *vi = ov_info(&m_vf, -1);
  assert(vi);
  m_info = {
    .numChannels = static_cast<NumChannels>(vi->channels),
    .bitsPerSample = 16,
    .sampleRate = static_cast<uint32_t>(vi->rate),
    .numSamples = static_cast<std::size_t>(ov_pcm_total(&m_vf, -1)),
  };
}
VorbisDecoder::~VorbisDecoder() { ov_clear(&m_vf); }

bool VorbisDecoder::isOpen() const {
  return m_dataStream && m_dataStream->isOpen();
}
const ClipInfo &VorbisDecoder::getInfo() const { return m_info; }

fsec VorbisDecoder::tell() {
  return fsec{isOpen() ? ov_time_tell(&m_vf) : 0.0};
}
void VorbisDecoder::seek(const fsec offset) {
  if (isOpen()) ov_time_seek(&m_vf, offset.count());
}
std::size_t VorbisDecoder::read(std::byte *buffer, const std::size_t length) {
  assert(buffer && length > 0);
  if (!isOpen()) return 0;

  std::size_t bytesDone{0};
  while (true) {
    int32_t currentSection{0};
    const auto decodeSize = ov_read(
      &m_vf, std::bit_cast<char *>(buffer) + bytesDone,
      static_cast<int32_t>(length - bytesDone), 0, 2, 1, &currentSection);
    if (decodeSize > 0) {
      bytesDone += decodeSize;
      if (bytesDone >= length) break;
    } else {
      break;
    }
  }

  // Mono, Stereo and 4-Channel files decode into the same channel order as
  // WAVEFORMATEXTENSIBLE, however 6-Channels files need to be re-ordered.
  if (m_info.numChannels == NumChannels::Surround5_1) {
    auto *samples = std::bit_cast<int16_t *>(buffer);
    for (auto i = 0u; i < (length / 2); i += 6) {
      //                             [0] [1] [2]  [3] [4] [5]
      // WAVEFORMATEXTENSIBLE Order : FL, FR,  C, LFE, RL, RR
      // OggVorbis Order            : FL,  C, FR,  RL, RR, LFE
      std::swap(samples[i + 1], samples[i + 2]); //  C -> FR
      std::swap(samples[i + 3], samples[i + 5]); // RL -> LFE
      std::swap(samples[i + 4], samples[i + 5]); // RR -> RL
    }
  }
  return bytesDone;
}

} // namespace audio
