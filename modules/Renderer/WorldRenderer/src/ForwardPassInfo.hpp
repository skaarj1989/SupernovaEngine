#pragma once

#include "BaseGeometryPassInfo.hpp"
#include "LightingPassFeatures.hpp"

namespace gfx {

struct ForwardPassInfo : BaseGeometryPassInfo {
  LightingPassFeatures features;
};

} // namespace gfx

namespace std {

template <> struct hash<gfx::ForwardPassInfo> {
  size_t operator()(const gfx::ForwardPassInfo &) const noexcept;
};

} // namespace std
