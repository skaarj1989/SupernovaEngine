#pragma once

#include "ShaderGraphCommon.hpp"

enum class EdgeType {
  Internal, // Parent<->Children connection.
  External,
};
struct EdgeProperty {
  EdgeID id{kInvalidId};
  EdgeType type{EdgeType::Internal};
};
