#include "renderer/VertexFormat.hpp"
#include "math/Hash.hpp"
#include <algorithm> // all_of, any_of
#include <cassert>

namespace std {

template <> struct hash<rhi::VertexAttribute> {
  auto operator()(const rhi::VertexAttribute &attribute) const noexcept {
    size_t h{0};
    hashCombine(h, attribute.type, attribute.offset);
    return h;
  }
};

} // namespace std

namespace gfx {

//
// VertexFormat class:
//

std::size_t VertexFormat::getHash() const { return m_hash; }

const rhi::VertexAttributes &VertexFormat::getAttributes() const {
  return m_attributes;
}
bool VertexFormat::contains(AttributeLocation location) const {
  return m_attributes.contains(int32_t(location));
}
bool VertexFormat::contains(
  std::initializer_list<AttributeLocation> locations) const {
  return std::ranges::all_of(
    std::cbegin(locations), std::cend(locations), [this](auto location) {
      return std::ranges::any_of(
        m_attributes.cbegin(), m_attributes.cend(),
        [v = int32_t(location)](const auto &p) { return p.first == v; });
    });
}

uint32_t VertexFormat::getStride() const { return m_stride; }

VertexFormat::VertexFormat(std::size_t hash, rhi::VertexAttributes &&attributes,
                           uint32_t stride)
    : m_hash{hash}, m_attributes{std::move(attributes)}, m_stride{stride} {}

//
// Builder:
//

using Builder = VertexFormat::Builder;

Builder &Builder::setAttribute(AttributeLocation location,
                               const rhi::VertexAttribute &attribute) {
  m_attributes.insert_or_assign(int32_t(location), attribute);
  return *this;
}

std::shared_ptr<VertexFormat> Builder::build() {
  std::size_t hash{0};

  uint32_t stride{0};
  for (const auto &[location, attribute] : m_attributes) {
    hashCombine(hash, location, attribute);
    stride += getSize(attribute.type);
  }

  if (const auto it = m_cache.find(hash); it != m_cache.cend()) {
    if (auto vertexFormat = it->second.lock(); vertexFormat)
      return vertexFormat;
  }

  auto vertexFormat = std::make_shared<VertexFormat>(
    VertexFormat{hash, std::move(m_attributes), stride});
  m_cache.insert_or_assign(hash, vertexFormat);

  return vertexFormat;
}

//
// Utility:
//

bool operator==(const VertexFormat &lhs, const VertexFormat &rhs) {
  return lhs.getHash() == rhs.getHash();
}

const char *toString(AttributeLocation location) {
#define CASE(Value)                                                            \
  case AttributeLocation::Value:                                               \
    return #Value

  switch (location) {
    CASE(Position);
    CASE(Color_0);
    CASE(Normal);
    CASE(TexCoord_0);
    CASE(TexCoord_1);
    CASE(Tangent);
    CASE(Bitangent);
    CASE(Joints);
    CASE(Weights);
  }
#undef CASE

  assert(false);
  return "Undefined";
}

std::vector<std::string> buildDefines(const VertexFormat &vertexFormat) {
  constexpr auto kMaxNumVertexDefines = 6;
  std::vector<std::string> defines;
  defines.reserve(kMaxNumVertexDefines);

  using enum AttributeLocation;

  if (vertexFormat.contains(Color_0)) defines.emplace_back("HAS_COLOR");
  if (vertexFormat.contains(Normal)) defines.emplace_back("HAS_NORMAL");
  if (vertexFormat.contains(TexCoord_0)) {
    defines.emplace_back("HAS_TEXCOORD0");
    if (vertexFormat.contains({Tangent, Bitangent})) {
      defines.emplace_back("HAS_TANGENTS");
    }
  }
  if (vertexFormat.contains(TexCoord_1)) defines.emplace_back("HAS_TEXCOORD1");
  if (vertexFormat.contains({Joints, Weights}))
    defines.emplace_back("IS_SKINNED");

  return defines;
}

bool isSkinned(const VertexFormat &vertexFormat) {
  using enum AttributeLocation;
  return vertexFormat.contains({Joints, Weights});
}

} // namespace gfx
