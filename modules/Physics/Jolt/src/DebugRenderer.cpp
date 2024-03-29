#include "physics/DebugRenderer.hpp"
#include "physics/Conversion.hpp"

namespace {

struct RenderPrimitive : JPH::RefTarget<RenderPrimitive> {
  uint32_t id{0};
};
struct BatchImpl : JPH::RefTargetVirtual, RenderPrimitive {
  explicit BatchImpl(uint32_t id_) { id = id_; }
  void AddRef() override { RenderPrimitive::AddRef(); }
  void Release() override {
    if (--mRefCount == 0) delete this;
  }
};

[[nodiscard]] auto convert(const JPH::DebugRenderer::Vertex &in) {
  constexpr auto kIgnorePointSize = 0.0f;
  return DebugDraw::Vertex{
    .position = glm::vec4{to_glm(in.mPosition), kIgnorePointSize},
    .color = in.mColor.GetUInt32(),
  };
}

} // namespace

//
// DebugRenderer class:
//

DebugRenderer::DebugRenderer() { JPH::DebugRenderer::Initialize(); }

void DebugRenderer::SetTarget(DebugDraw &dd) { m_debugDraw = &dd; }
void DebugRenderer::Submit() {
  assert(m_debugDraw);

  DebugDraw::MeshBatch meshBatch{.registry = &m_meshes};
  for (const auto &[geometry, instances] : m_instanceMap) {
    const auto *batch = static_cast<const BatchImpl *>(
      geometry->mLODs.begin()->mTriangleBatch.GetPtr());
    meshBatch.add(&m_meshes.getTriangleMesh(batch->id), instances);
  }
  m_debugDraw->set(meshBatch);

  m_instanceMap.clear();
}

void DebugRenderer::DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to,
                             JPH::ColorArg color) {
  m_debugDraw->addLine(to_glm(from), to_glm(to), color.GetUInt32());
}
void DebugRenderer::DrawTriangle(JPH::Vec3Arg v0, JPH::Vec3Arg v1,
                                 JPH::Vec3Arg v2, JPH::ColorArg color,
                                 ECastShadow) {
  const auto u32 = color.GetUInt32();
  m_debugDraw->addLine(to_glm(v0), to_glm(v1), u32)
    .addLine(to_glm(v1), to_glm(v2), u32)
    .addLine(to_glm(v2), to_glm(v0), u32);
}
JPH::DebugRenderer::Batch
DebugRenderer::CreateTriangleBatch(const Triangle *triangles, int32_t count) {
  assert(triangles != nullptr && count > 0);

  DebugDraw::VertexList vertices;
  vertices.reserve(count * 3);
  for (const auto &triangle :
       std::span{triangles, static_cast<std::size_t>(count)}) {
    for (auto i = 0; i < 3; ++i)
      vertices.emplace_back(convert(triangle.mV[i]));
  }

  return new BatchImpl{m_meshes.create(vertices)};
}
JPH::DebugRenderer::Batch DebugRenderer::CreateTriangleBatch(
  const Vertex *inVertices, int32_t numVertices, const uint32_t *inIndices,
  int32_t numIndices) {
  assert(inVertices != nullptr && numVertices > 0);

  DebugDraw::VertexList vertices;
  vertices.reserve(numVertices);
  for (const auto &vertex :
       std::span{inVertices, static_cast<std::size_t>(numVertices)}) {
    vertices.emplace_back(convert(vertex));
  }

  return new BatchImpl{m_meshes.create(
    vertices, std::span{inIndices, static_cast<std::size_t>(numIndices)})};
}
void DebugRenderer::DrawGeometry(JPH::Mat44Arg modelMatrix, const JPH::AABox &,
                                 [[maybe_unused]] float LODScaleSq,
                                 JPH::ColorArg color,
                                 const GeometryRef &geometry, ECullMode,
                                 ECastShadow, EDrawMode) {
  m_instanceMap[geometry].emplace_back(to_glm(modelMatrix),
                                       to_glm(color.ToVec4()));
}
void DebugRenderer::DrawText3D([[maybe_unused]] JPH::Vec3Arg position,
                               const std::string_view &, JPH::ColorArg,
                               [[maybe_unused]] float height) {
  // ...
}
