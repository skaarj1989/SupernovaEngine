#pragma once

#include "Jolt/Jolt.h"
#include "Jolt/Renderer/DebugRenderer.h"

#include "DebugDraw.hpp"

class DebugRenderer final : public JPH::DebugRenderer {
public:
  DebugRenderer();

  void SetTarget(DebugDraw &);
  void Submit();

  void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo,
                JPH::ColorArg inColor) override;

  void DrawTriangle(JPH::Vec3Arg v0, JPH::Vec3Arg v1, JPH::Vec3Arg v2,
                    JPH::ColorArg, ECastShadow) override;
  Batch CreateTriangleBatch(const Triangle *, int32_t count) override;
  Batch CreateTriangleBatch(const Vertex *, int32_t numVertices,
                            const uint32_t *indices,
                            int32_t numIndices) override;
  void DrawGeometry(JPH::Mat44Arg, const JPH::AABox &, float LODScaleSq,
                    JPH::ColorArg, const GeometryRef &, ECullMode, ECastShadow,
                    EDrawMode) override;
  void DrawText3D(JPH::Vec3Arg position, const std::string_view &,
                  JPH::ColorArg, float height) override;

private:
  DebugDraw *m_debugDraw{nullptr};

  DebugDraw::MeshRegistry m_meshes;
  using InstanceMap = std::unordered_map<JPH::DebugRenderer::GeometryRef,
                                         DebugDraw::InstanceList>;
  InstanceMap m_instanceMap;
};
