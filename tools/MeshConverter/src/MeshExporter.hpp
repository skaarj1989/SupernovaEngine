#pragma once

#include "Mesh.hpp"
#include "AnimationConverter.hpp"

class MeshExporter {
public:
  enum class Flags {
    None = 0,
    IgnoreMaterials = 1 << 0,
    GenerateLODs = 1 << 1,
  };
  explicit MeshExporter(Flags);
  MeshExporter(const MeshExporter &) = delete;
  MeshExporter &operator=(const MeshExporter &) = delete;

  MeshExporter &load(const aiScene *);
  MeshExporter &save(const std::filesystem::path &p);

private:
  void _prepareSkeleton(const aiScene *);
  void _processMeshes(std::span<aiMesh *>, std::span<aiMaterial *>);
  void _findVertexFormat(std::span<aiMesh *>);
  [[nodiscard]] const offline::SubMesh &_processMesh(const aiMesh &,
                                                     std::span<aiMaterial *>);
  void _buildBufferOffsets();

  void _fillVertexBuffer(const aiMesh &, offline::SubMesh &);
  void _fillIndexBuffer(const aiMesh &, offline::SubMesh &);
  void _generateLODs(offline::SubMesh &, std::size_t numLevels);

  bool _writeDataBuffer(const std::filesystem::path &dir) const;
  bool _exportMeshMeta(const std::filesystem::path &p) const;

private:
  Flags m_flags{Flags::None};

  struct Transforms {
    Transforms() = default;
    explicit Transforms(const aiMatrix4x4 &m)
        : root{m}, inverseRoot{aiMatrix4x4{root}.Inverse()},
          normalMatrix{aiMatrix3x3{aiMatrix4x4{inverseRoot}.Transpose()}} {}

    aiMatrix4x4 root;
    aiMatrix4x4 inverseRoot;
    aiMatrix3x3 normalMatrix;
  };
  Transforms m_transforms;

  offline::Mesh m_mesh;
  RuntimeSkeleton m_runtimeSkeleton;
  AnimationList m_animations;
};

template <> struct has_flags<MeshExporter::Flags> : std::true_type {};
