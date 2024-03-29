#pragma once

#include <cassert>

#define VK_CHECK(x)                                                            \
  do {                                                                         \
    [[maybe_unused]] VkResult result = x;                                      \
    assert(result == VK_SUCCESS);                                              \
  } while (false)
