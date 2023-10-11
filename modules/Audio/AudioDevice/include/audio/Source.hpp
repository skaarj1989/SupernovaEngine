#pragma once

#include "Buffer.hpp"
#include "SourceBase.hpp"

namespace audio {

class Source final : public SourceBase {
public:
  explicit Source(std::shared_ptr<Buffer> = nullptr);
  Source(const Source &) = delete;
  Source(Source &&) noexcept = default;
  ~Source() override;

  Source &operator=(const Source &) = delete;
  Source &operator=(Source &&) noexcept = default;

  void setBuffer(std::shared_ptr<Buffer>);
  void setLooping(const bool);

  [[nodiscard]] std::shared_ptr<Buffer> getBuffer() const;
  [[nodiscard]] bool isLooping() const;

  [[nodiscard]] fsec tell() const;
  void seek(const fsec offset);

private:
  std::shared_ptr<Buffer> m_buffer;
};

} // namespace audio
