#pragma once

#include <cassert>

#define VK_CHECK(x)                                                            \
  do {                                                                         \
    VkResult result = x;                                                       \
    assert(result == VK_SUCCESS);                                              \
  } while (false)
