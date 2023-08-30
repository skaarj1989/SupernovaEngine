#include "VertexInfo.hpp"
#include <cassert>

//
// Builder:
//

using Builder = VertexInfo::Builder;

Builder &Builder::add(gfx::AttributeLocation location,
                      rhi::VertexAttribute::Type type) {
  if (auto &&[_, inserted] =
        m_attributes.try_emplace(location, type, m_currentOffset);
      inserted) {
    m_currentOffset += rhi::getSize(type);
  }
  return *this;
}

VertexInfo Builder::build() {
  assert(m_currentOffset > 0 && !m_attributes.empty());
  return VertexInfo{m_currentOffset, std::move(m_attributes)};
}

//
// VertexInfo:
//

uint32_t VertexInfo::getStride() const {
  assert(m_stride > 0);
  return m_stride;
}
const rhi::VertexAttribute &
VertexInfo::getAttribute(gfx::AttributeLocation location) const {
  return m_attributes.at(location);
}
const AttributesMap &VertexInfo::getAttributes() const { return m_attributes; }

VertexInfo::VertexInfo(uint32_t stride, AttributesMap &&attributes)
    : m_stride{stride}, m_attributes{std::move(attributes)} {}
