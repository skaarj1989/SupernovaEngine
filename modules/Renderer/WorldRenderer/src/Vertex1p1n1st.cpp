#include "renderer/Vertex1p1n1st.hpp"

namespace gfx {

std::shared_ptr<VertexFormat> Vertex1p1n1st::getVertexFormat() {
  return VertexFormat::Builder{}
    .setAttribute(AttributeLocation::Position,
                  {
                    .type = rhi::VertexAttribute::Type::Float3,
                    .offset = 0,
                  })
    .setAttribute(AttributeLocation::Normal,
                  {
                    .type = rhi::VertexAttribute::Type::Float3,
                    .offset = offsetof(Vertex1p1n1st, normal),
                  })
    .setAttribute(AttributeLocation::TexCoord_0,
                  {
                    .type = rhi::VertexAttribute::Type::Float2,
                    .offset = offsetof(Vertex1p1n1st, texCoord),
                  })
    .build();
}

} // namespace gfx
