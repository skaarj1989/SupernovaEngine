#pragma once

#include "os/DataStream.hpp"
#include "minizip/unzip.h"

namespace os {

class InZipFile final : public DataStream {
public:
  InZipFile() = delete;
  InZipFile(unzFile archive, uint64_t offset, uint64_t size);
  InZipFile(const InZipFile &) = delete;
  InZipFile(InZipFile &&) noexcept = delete;
  ~InZipFile() override;

  InZipFile &operator=(const InZipFile &) = delete;
  InZipFile &operator=(InZipFile &&) noexcept = delete;

  int32_t close() override;

  bool isOpen() const override;
  std::size_t getSize() const override;

  std::size_t tell() const override;
  std::size_t seek(std::size_t offset, Origin) override;
  std::size_t read(void *buffer, std::size_t length) override;

private:
  unzFile m_archive;
  uint64_t m_startOffset{0};
  uint64_t m_uncompressedSize{0};
};

} // namespace os
