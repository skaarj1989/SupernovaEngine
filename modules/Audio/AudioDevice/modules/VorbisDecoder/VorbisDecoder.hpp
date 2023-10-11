#pragma once

#include "Decoder.hpp"
#include "os/DataStream.hpp"
#include "vorbis/vorbisfile.h"

namespace audio {

class VorbisDecoder final : public Decoder {
public:
  explicit VorbisDecoder(std::unique_ptr<os::DataStream> &&);
  ~VorbisDecoder() override;

  bool isOpen() const override;
  const ClipInfo &getInfo() const override;

  fsec tell() override;
  void seek(const fsec offset) override;
  std::size_t read(std::byte *buffer, const std::size_t length) override;

private:
  ClipInfo m_info{};
  std::unique_ptr<os::DataStream> m_dataStream;
  OggVorbis_File m_vf{};
};

} // namespace audio
