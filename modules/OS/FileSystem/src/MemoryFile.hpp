#pragma once

#include "os/DataStream.hpp"

namespace os {

class MemoryFile final : public DataStream {
public:
  MemoryFile() = delete;
  MemoryFile(char *data, std::size_t length);
  MemoryFile(const MemoryFile &) = delete;
  MemoryFile(MemoryFile &&) noexcept = delete;
  ~MemoryFile() override;

  MemoryFile &operator=(const MemoryFile &) = delete;
  MemoryFile &operator=(MemoryFile &&) noexcept = delete;

  int32_t close() override;

  bool isOpen() const override;
  std::size_t getSize() const override;

  std::size_t tell() const override;
  std::size_t seek(std::size_t offset, Origin) override;
  std::size_t read(void *buffer, std::size_t length) override;

private:
  char *m_filePtr{nullptr};
  char *m_currentPos{nullptr};
  std::size_t m_size{0};
};

} // namespace os
