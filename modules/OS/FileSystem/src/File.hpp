#pragma once

#include "os/DataStream.hpp"
#include <filesystem>

namespace os {

class PhysicalFile final : public DataStream {
public:
  PhysicalFile() = delete;
  explicit PhysicalFile(const std::filesystem::path &);
  PhysicalFile(const PhysicalFile &) = delete;
  PhysicalFile(PhysicalFile &&) noexcept = delete;
  ~PhysicalFile() override;

  PhysicalFile &operator=(const PhysicalFile &) = delete;
  PhysicalFile &operator=(PhysicalFile &&) noexcept = delete;

  int32_t close() override;

  bool isOpen() const override;
  std::size_t getSize() const override;

  std::size_t tell() const override;
  std::size_t seek(std::size_t offset, Origin) override;
  std::size_t read(void *buffer, std::size_t length) override;

private:
  void _fetchSize();

private:
  FILE *m_file{nullptr};
  std::size_t m_size{0};
};

} // namespace os
