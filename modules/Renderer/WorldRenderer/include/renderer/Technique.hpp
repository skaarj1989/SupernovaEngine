#pragma once

#include "PipelineGroups.hpp"
#include <cstdint>

namespace gfx {

class Technique {
public:
  virtual ~Technique() = default;

  virtual uint32_t count(const PipelineGroups) const = 0;
  virtual void clear(const PipelineGroups) = 0;
};

} // namespace gfx
