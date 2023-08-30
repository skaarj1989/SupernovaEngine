#pragma once

#include "ShaderGraphCommon.hpp"

struct IDPair {
  VertexDescriptor vd{nullptr};
  int32_t id{kInvalidId};

  [[nodiscard]] auto isValid() const {
    return vd != nullptr && id != kInvalidId;
  }
};
