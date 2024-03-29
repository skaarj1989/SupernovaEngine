#pragma once

#include "SourceBase.hpp"
#include "Decoder.hpp"

namespace audio {

class Device;

class StreamPlayer final : public SourceBase {
  friend class Device;

public:
  explicit StreamPlayer(const int32_t numBuffers = 3);
  StreamPlayer(const StreamPlayer &) = delete;
  StreamPlayer(StreamPlayer &&) noexcept = default;
  ~StreamPlayer() override;

  StreamPlayer &operator=(const StreamPlayer &) = delete;
  StreamPlayer &operator=(StreamPlayer &&) = default;

  void setStream(std::unique_ptr<Decoder> &&, const fsec bufferDuration);
  void setLooping(const bool);

  [[nodiscard]] fsec getDuration() const;
  [[nodiscard]] bool isLooping() const;

  [[nodiscard]] fsec tell() const;
  void seek(const fsec offset);

  // Function hiding for the following functions is on purpose!
  // A reference/pointer to SourceBase is never used.

  void play();
  void pause();
  void stop();

  void update();

private:
  void _start();
  [[nodiscard]] uint32_t _read();

private:
  std::vector<ALuint> m_buffers;

  std::unique_ptr<Decoder> m_decoder;
  ALenum m_format{AL_NONE};
  std::vector<std::byte> m_stagingBuffer;
  bool m_looping{false};
};

} // namespace audio
