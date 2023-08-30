#include "Jolt/Jolt.h"
#include "physics/DebugRenderer.hpp"
#include "physics/Conversion.hpp"
#include "math/Color.hpp"
#include "DebugDraw.hpp"

DebugRenderer::DebugRenderer() { JPH::DebugRenderer::Initialize(); }

void DebugRenderer::setSource(DebugDraw &dd) { m_debugDraw = &dd; }

void DebugRenderer::DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to,
                             JPH::ColorArg color) {
  m_debugDraw->addLine(to_glm(from), to_glm(to), color.GetUInt32());
}
void DebugRenderer::DrawTriangle(JPH::Vec3Arg v0, JPH::Vec3Arg v1,
                                 JPH::Vec3Arg v2, JPH::ColorArg color,
                                 ECastShadow) {
  const auto u32 = color.GetUInt32();
  m_debugDraw->addLine(to_glm(v0), to_glm(v1), u32);
  m_debugDraw->addLine(to_glm(v1), to_glm(v2), u32);
  m_debugDraw->addLine(to_glm(v2), to_glm(v0), u32);
}
JPH::DebugRenderer::Batch
DebugRenderer::CreateTriangleBatch(const Triangle *triangle, int32_t count) {
  return {};
}
JPH::DebugRenderer::Batch
DebugRenderer::CreateTriangleBatch(const Vertex *, int32_t numVertices,
                                   const uint32_t *indices,
                                   int32_t numIndices) {
  return {};
}
void DebugRenderer::DrawGeometry(JPH::Mat44Arg modelMatrix,
                                 const JPH::AABox &box, float LODScaleSq,
                                 JPH::ColorArg, const GeometryRef &geometry,
                                 ECullMode, ECastShadow, EDrawMode) {
#if 1
  m_debugDraw->addAABB(
    from_Jolt(geometry.GetPtr()->mBounds.Transformed(modelMatrix)));
#else
  m_debugDraw->addAABB(from_Jolt(box));
#endif
}
void DebugRenderer::DrawText3D(JPH::Vec3Arg position, const std::string_view &,
                               JPH::ColorArg, float height) {
  // ...
}
