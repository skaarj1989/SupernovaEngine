#pragma once

#include "rhi/VertexAttributes.hpp"
#include "robin_hood.h"

namespace gfx {

// NOTE: Make sure that the following locations match with the ones in the file:
// shaders/Mesh.vert
enum class AttributeLocation : int32_t {
  Position = 0,
  Color_0,
  Normal,
  TexCoord_0,
  TexCoord_1,
  Tangent,
  Bitangent,

  Joints,
  Weights,
};

class VertexFormat final {
public:
  VertexFormat() = delete;
  VertexFormat(const VertexFormat &) = delete;
  VertexFormat(VertexFormat &&) noexcept = default;
  ~VertexFormat() = default;

  VertexFormat &operator=(const VertexFormat &) = delete;
  VertexFormat &operator=(VertexFormat &&) noexcept = default;

  [[nodiscard]] std::size_t getHash() const;

  [[nodiscard]] const rhi::VertexAttributes &getAttributes() const;
  [[nodiscard]] bool contains(AttributeLocation) const;
  [[nodiscard]] bool contains(std::initializer_list<AttributeLocation>) const;

  [[nodiscard]] uint32_t getStride() const;

  friend bool operator==(const VertexFormat &, const VertexFormat &);

  class Builder {
  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) noexcept = delete;

    Builder &setAttribute(AttributeLocation, const rhi::VertexAttribute &);

    [[nodiscard]] std::shared_ptr<VertexFormat> build();

  private:
    rhi::VertexAttributes m_attributes;

    using Cache =
      robin_hood::unordered_map<std::size_t, std::weak_ptr<VertexFormat>>;
    inline static Cache m_cache;
  };

private:
  VertexFormat(std::size_t hash, rhi::VertexAttributes &&, uint32_t stride);

private:
  const std::size_t m_hash{0u};
  const rhi::VertexAttributes m_attributes;
  const uint32_t m_stride{0};
};

[[nodiscard]] const char *toString(AttributeLocation);

[[nodiscard]] std::vector<std::string> buildDefines(const VertexFormat &);
[[nodiscard]] bool isSkinned(const VertexFormat &);

} // namespace gfx
