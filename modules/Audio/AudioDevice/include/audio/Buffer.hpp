#pragma once

#include "AL/al.h"
#include "ClipInfo.hpp"

namespace audio {

class Device;
class Source;

class Buffer {
  friend class Device;
  friend class Source;

public:
  Buffer() = delete;
  Buffer(const Buffer &) = delete;
  Buffer(Buffer &&) noexcept;
  ~Buffer();

  Buffer &operator=(const Buffer &) = delete;
  Buffer &operator=(Buffer &&) noexcept;

  const ClipInfo &getInfo() const;

private:
  Buffer(const ClipInfo &, const void *data);
  void _destroy();

private:
  ALuint m_id{AL_NONE};
  ClipInfo m_info;
};

} // namespace audio
