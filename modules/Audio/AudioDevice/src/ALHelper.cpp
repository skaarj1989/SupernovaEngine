#include "ALHelper.hpp"
#include <cassert>

namespace audio {

namespace {

[[nodiscard]] constexpr auto makeHash(const NumChannels numChannels,
                                      const uint8_t bitsPerSample) {
  return std::to_underlying(numChannels) << 8 | bitsPerSample;
}

} // namespace

ALenum pickFormat(const ClipInfo &clipInfo) {
  switch (makeHash(clipInfo.numChannels, clipInfo.bitsPerSample)) {
    using enum NumChannels;

  case makeHash(Mono, 8):
    return AL_FORMAT_MONO8;
  case makeHash(Mono, 16):
    return AL_FORMAT_MONO16;
  case makeHash(Mono, 32):
    return alGetEnumValue("AL_FORMAT_MONO_FLOAT32");
  case makeHash(Mono, 64):
    return alGetEnumValue("AL_FORMAT_MONO_DOUBLE_EXT");

  case makeHash(Stereo, 8):
    return AL_FORMAT_STEREO8;
  case makeHash(Stereo, 16):
    return AL_FORMAT_STEREO16;
  case makeHash(Stereo, 32):
    return alGetEnumValue("AL_FORMAT_STEREO_FLOAT32");
  case makeHash(Stereo, 64):
    return alGetEnumValue("AL_FORMAT_STEREO_DOUBLE_EXT");

  case makeHash(QuadSurround, 8):
    return alGetEnumValue("AL_FORMAT_QUAD8");
  case makeHash(QuadSurround, 16):
    return alGetEnumValue("AL_FORMAT_QUAD16");
  case makeHash(QuadSurround, 32):
    return alGetEnumValue("AL_FORMAT_QUAD32");

  case makeHash(Surround5_1, 8):
    return alGetEnumValue("AL_FORMAT_51CHN8");
  case makeHash(Surround5_1, 16):
    return alGetEnumValue("AL_FORMAT_51CHN16");
  case makeHash(Surround5_1, 32):
    return alGetEnumValue("AL_FORMAT_51CHN32");

  case makeHash(Surround6_1, 8):
    return alGetEnumValue("AL_FORMAT_61CHN8");
  case makeHash(Surround6_1, 16):
    return alGetEnumValue("AL_FORMAT_61CHN16");
  case makeHash(Surround6_1, 32):
    return alGetEnumValue("AL_FORMAT_61CHN32");

  case makeHash(Surround7_1, 8):
    return alGetEnumValue("AL_FORMAT_71CHN8");
  case makeHash(Surround7_1, 16):
    return alGetEnumValue("AL_FORMAT_71CHN16");
  case makeHash(Surround7_1, 32):
    return alGetEnumValue("AL_FORMAT_71CHN32");
  }

  assert(false);
  return AL_INVALID;
}

} // namespace audio
