#pragma once

#include "renderer/VertexFormat.hpp"

using AttributesMap = std::map<gfx::AttributeLocation, rhi::VertexAttribute>;

class VertexInfo {
public:
  VertexInfo() = default;
  VertexInfo(const VertexInfo &) = delete;
  VertexInfo(VertexInfo &&) noexcept = default;
  ~VertexInfo() = default;

  VertexInfo &operator=(const VertexInfo &) = delete;
  VertexInfo &operator=(VertexInfo &&) = default;

  [[nodiscard]] uint32_t getStride() const;
  [[nodiscard]] const rhi::VertexAttribute &
    getAttribute(gfx::AttributeLocation) const;
  [[nodiscard]] const AttributesMap &getAttributes() const;

  class Builder {
  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) = delete;

    Builder &add(gfx::AttributeLocation, rhi::VertexAttribute::Type);

    [[nodiscard]] VertexInfo build();

  private:
    uint32_t m_currentOffset{0};
    AttributesMap m_attributes;
  };

private:
  VertexInfo(uint32_t stride, AttributesMap &&);

private:
  uint32_t m_stride{0};
  AttributesMap m_attributes;
};
