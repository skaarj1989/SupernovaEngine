#include "glad/vulkan.h"
#include "fmt/printf.h"
#include "spdlog/spdlog.h"

namespace {

// https://github.com/gabime/spdlog/issues/1843

template <class Logger, class... Args>
void _printf(Logger logger, spdlog::level::level_enum level,
             spdlog::source_loc loc, const char *fmt,
             const Args &...args) noexcept {
  if (logger && logger->should_log(level))
    logger->log(loc, level, "{}", fmt::sprintf(fmt, args...));
}

} // namespace

#define SPDLOG_LOGGER_PRINTF(logger, level, ...)                               \
  _printf(logger, level,                                                       \
          spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},             \
          __VA_ARGS__)

#define VMA_DEBUG_LOG(...)                                                     \
  SPDLOG_LOGGER_PRINTF(spdlog::get("VMA"), spdlog::level::trace, __VA_ARGS__)

#define VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT                          \
  VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT

#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_IMPLEMENTATION

#pragma warning(push, 0)
#include "vk_mem_alloc.h"
#pragma warning(pop)
