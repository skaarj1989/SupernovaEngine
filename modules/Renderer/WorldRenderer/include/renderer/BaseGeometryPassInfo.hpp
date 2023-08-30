#pragma once

#include "rhi/PrimitiveTopology.hpp"
#include "VertexFormat.hpp"
#include "Material.hpp"

namespace gfx {

struct BaseGeometryPassInfo {
  rhi::PixelFormat depthFormat;
  std::vector<rhi::PixelFormat> colorFormats;
  rhi::PrimitiveTopology topology{rhi::PrimitiveTopology::TriangleList};
  const VertexFormat *vertexFormat{nullptr};
  const Material *material{nullptr};
};

} // namespace gfx

namespace std {

template <> struct hash<gfx::BaseGeometryPassInfo> {
  std::size_t operator()(const gfx::BaseGeometryPassInfo &) const noexcept;
};

} // namespace std
