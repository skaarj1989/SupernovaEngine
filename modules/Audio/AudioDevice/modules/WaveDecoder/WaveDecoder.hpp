#pragma once

#include "Decoder.hpp"
#include "os/DataStream.hpp"

namespace audio {

class WaveDecoder final : public Decoder {
public:
  explicit WaveDecoder(std::unique_ptr<os::DataStream> &&);
  ~WaveDecoder() override = default;

  bool isOpen() const override;
  const ClipInfo &getInfo() const override;

  fsec tell() override;
  void seek(const fsec offset) override;
  std::size_t read(std::byte *buffer, const std::size_t length) override;

private:
  void _readInfo();

  void _setSampleOffset(const std::size_t offset);
  [[nodiscard]] std::size_t _getSampleOffset() const;

private:
  std::unique_ptr<os::DataStream> m_dataStream;
  ClipInfo m_info{};
  std::size_t m_dataOffset{0};
};

} // namespace audio
