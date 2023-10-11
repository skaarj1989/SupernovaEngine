#pragma once

#include "ClipInfo.hpp"
#include <cstddef> // byte

namespace audio {

class Decoder {
public:
  virtual ~Decoder() = default;

  virtual bool isOpen() const = 0;
  virtual const ClipInfo &getInfo() const = 0;

  // @return Current position in seconds.
  virtual fsec tell() = 0;
  // @return true, if the stream has reached its end.
  [[nodiscard]] bool eos() { return tell() >= getInfo().duration(); }
  // @param offset Target position in seconds.
  virtual void seek(const fsec offset) = 0;
  virtual void rewind() { seek(fsec{0}); }

  // @param buffer Pointer to the array where the read bytes are stored.
  // @param length The number of bytes to be read.
  // @return Number of bytes read successfully, which may be less than the given
  // length.
  virtual std::size_t read(std::byte *buffer, const std::size_t length) = 0;
};

} // namespace audio
