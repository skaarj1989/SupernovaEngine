#pragma once

#include "os/DataStream.hpp"
#include "ozz/base/io/stream.h"
#include <memory>

class OzzFileWrapper final : public ozz::io::Stream {
public:
  explicit OzzFileWrapper(std::unique_ptr<os::DataStream> &&);
  ~OzzFileWrapper() override = default;

  bool opened() const override;

  size_t Read(void *_buffer, size_t _size) override;
  size_t Write(const void *_buffer, size_t _size) override;
  int Seek(int _offset, Origin _origin) override;
  int Tell() const override;
  size_t Size() const override;

private:
  std::unique_ptr<os::DataStream> m_stream;
};
