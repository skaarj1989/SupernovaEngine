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

  using Hash = std::size_t;
  using VertexStride = uint32_t;

  [[nodiscard]] Hash getHash() const;

  [[nodiscard]] const rhi::VertexAttributes &getAttributes() const;
  [[nodiscard]] bool contains(const AttributeLocation) const;
  [[nodiscard]] bool contains(std::initializer_list<AttributeLocation>) const;

  [[nodiscard]] VertexStride getStride() const;

  friend bool operator==(const VertexFormat &, const VertexFormat &);

  class Builder {
  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) noexcept = delete;

    Builder &setAttribute(const AttributeLocation,
                          const rhi::VertexAttribute &);

    [[nodiscard]] std::shared_ptr<VertexFormat> build();

  private:
    rhi::VertexAttributes m_attributes;

    using Cache = robin_hood::unordered_map<Hash, std::weak_ptr<VertexFormat>>;
    inline static Cache m_cache;
  };

private:
  VertexFormat(const Hash, rhi::VertexAttributes &&, const VertexStride);

private:
  const Hash m_hash{0u};
  const rhi::VertexAttributes m_attributes;
  const VertexStride m_stride{0};
};

[[nodiscard]] const char *toString(const AttributeLocation);

[[nodiscard]] std::vector<std::string> buildDefines(const VertexFormat &);
[[nodiscard]] bool isSkinned(const VertexFormat &);

} // namespace gfx
