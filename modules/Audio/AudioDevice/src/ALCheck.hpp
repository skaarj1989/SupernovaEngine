#pragma once

#include <cassert>

#define AL_CHECK(fn)                                                           \
  do {                                                                         \
    ##fn;                                                                      \
    assert(alGetError() == AL_NO_ERROR);                                       \
  } while (false)
