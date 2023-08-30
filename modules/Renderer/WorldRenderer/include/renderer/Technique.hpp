#pragma once

#include "PipelineGroups.hpp"

namespace gfx {

class Technique {
public:
  virtual ~Technique() = default;

  virtual uint32_t count(PipelineGroups) const = 0;
  virtual void clear(PipelineGroups) = 0;
};

} // namespace gfx
