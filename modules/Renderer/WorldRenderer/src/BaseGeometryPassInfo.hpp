#pragma once

#include "rhi/PixelFormat.hpp"
#include "rhi/PrimitiveTopology.hpp"
#include <functional>
#include <vector>

namespace gfx {

class VertexFormat;
class Material;

struct BaseGeometryPassInfo {
  rhi::PixelFormat depthFormat;
  std::vector<rhi::PixelFormat> colorFormats;
  rhi::PrimitiveTopology topology{rhi::PrimitiveTopology::TriangleList};
  const VertexFormat *vertexFormat{nullptr};
  const Material *material{nullptr};
  bool writeUserData{false};
};

} // namespace gfx

namespace std {

template <> struct hash<gfx::BaseGeometryPassInfo> {
  size_t operator()(const gfx::BaseGeometryPassInfo &) const noexcept;
};

} // namespace std
