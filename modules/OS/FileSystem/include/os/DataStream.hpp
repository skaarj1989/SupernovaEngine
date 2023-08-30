#pragma once

#include <cstdint>
#include <cstdlib>

namespace os {

class DataStream {
public:
  enum class Origin : int32_t { Beginning = 0, Current, End };

  virtual ~DataStream() = default;

  virtual int32_t close() = 0;

  [[nodiscard]] virtual bool isOpen() const = 0;
  /** @return Raw size (uncompressed) */
  [[nodiscard]] virtual std::size_t getSize() const = 0;

  /** @return Current value of the data position indicator */
  [[nodiscard]] virtual std::size_t tell() const = 0;
  /**
   * Sets the data position indicator
   * @param offset Shift the position relative to the Origin
   */
  virtual std::size_t seek(std::size_t offset, Origin) = 0;
  /**
   * @param [out] buffer Pointer to the first element in the array to be read
   * @param length Size in bytes to be read
   * @return Number of elements read successfully
   */
  virtual std::size_t read(void *buffer, std::size_t length) = 0;

  /**
   * Convenience function, moves the data position indicator to the beginning.
   */
  inline void rewind() { seek(0, Origin::Beginning); }
};

} // namespace os
