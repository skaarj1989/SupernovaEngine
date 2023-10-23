#include "physics/ColliderLoader.hpp"

#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"

#include "physics/Conversion.hpp"

#include "math/json.hpp"

#include "os/FileSystem.hpp"
#include "spdlog/spdlog.h"

using namespace nlohmann;

#define MAKE_PAIR(Value)                                                       \
  { ColliderType::Value, #Value }

NLOHMANN_JSON_SERIALIZE_ENUM(ColliderType, {
                                             MAKE_PAIR(Box),
                                             MAKE_PAIR(Sphere),
                                             MAKE_PAIR(Capsule),
                                             MAKE_PAIR(ConvexHull),
                                             MAKE_PAIR(TriangleMesh),
                                           });

#undef MAKE_PAIR

struct MeshInfo {
  struct VertexInfo {
    uint32_t byteOffset;
    uint32_t count;
    uint32_t stride;
  };
  struct IndexInfo {
    uint32_t byteOffset;
    uint32_t count;
    uint32_t stride;
  };
  VertexInfo vertices;
  IndexInfo indices;

  struct SubMeshInfo {
    uint32_t vertexOffset;
    uint32_t numVertices;
    struct LOD {
      uint32_t indexOffset;
      uint32_t numIndices;
    };
    std::vector<LOD> LODs;
  };
  std::vector<SubMeshInfo> subMeshes;
};
static void from_json(const json &j, MeshInfo::VertexInfo &out) {
  j.at("byteOffset").get_to(out.byteOffset);
  j.at("count").get_to(out.count);
  j.at("stride").get_to(out.stride);
}
static void from_json(const json &j, MeshInfo::IndexInfo &out) {
  j.at("byteOffset").get_to(out.byteOffset);
  j.at("count").get_to(out.count);
  j.at("stride").get_to(out.stride);
}
static void from_json(const json &j, MeshInfo::SubMeshInfo::LOD &out) {
  j.at("indexOffset").get_to(out.indexOffset);
  j.at("numIndices").get_to(out.numIndices);
}
static void from_json(const json &j, MeshInfo::SubMeshInfo &out) {
  j.at("vertexOffset").get_to(out.vertexOffset);
  j.at("numVertices").get_to(out.numVertices);
  j.at("LOD").get_to(out.LODs);
}
static void from_json(const json &j, MeshInfo &out) {
  j.at("vertices").get_to(out.vertices);
  j.at("indices").get_to(out.indices);
  j.at("meshes").get_to(out.subMeshes);
}

struct Mesh {
  MeshInfo info;
  std::unique_ptr<std::byte[]> buffer;
};

[[nodiscard]] Mesh readMesh(const std::filesystem::path &p) {
  const auto j = json::parse(*os::FileSystem::readText(p));

  const auto dir = p.parent_path();
  const auto relativePath = j.at("buffer").get<std::string>();
  auto buffer = os::FileSystem::readBuffer(dir / relativePath);
  if (!buffer) throw std::runtime_error{buffer.error()};

  return Mesh{
    .info = j,
    .buffer = std::move(buffer->data),
  };
}

[[nodiscard]] auto convert(const std::byte *buffer, uint32_t stride) {
#define INDEXED_TRIANGLE(Type)                                                 \
  JPH::IndexedTriangle {                                                       \
    *std::bit_cast<Type *>(buffer),                                            \
      *std::bit_cast<Type *>(buffer + sizeof(Type)),                           \
      *std::bit_cast<Type *>(buffer + (2 * sizeof(Type)))                      \
  }

  switch (stride) {
  case 2:
    return INDEXED_TRIANGLE(uint16_t);
  case 4:
    return INDEXED_TRIANGLE(uint32_t);

#undef INDEXED_TRIANGLE
  default:
    assert(false);
  }
  return JPH::IndexedTriangle{};
}

[[nodiscard]] auto getShapeSettings(const Mesh &mesh) {
  const auto &[meta, buffer] = mesh;

  JPH::VertexList vertices;
  vertices.reserve(meta.vertices.count);
  JPH::IndexedTriangleList triangles;
  triangles.reserve(meta.indices.count / 3);

  const auto vertexStride = meta.vertices.stride;
  const auto indexStride = meta.indices.stride;

  for (const auto &subMesh : meta.subMeshes) {
    const auto baseVertexIdx =
      meta.vertices.byteOffset + (subMesh.vertexOffset * vertexStride);
    for (auto i = 0; i < subMesh.numVertices; ++i) {
      const auto vertex = &buffer[baseVertexIdx + (i * vertexStride)];
      vertices.push_back(*std::bit_cast<const JPH::Float3 *>(vertex));
    }

    const auto &lod = subMesh.LODs.front();
    const auto baseIndexIdx =
      meta.indices.byteOffset + (lod.indexOffset * indexStride);

    const auto numTriangles = lod.numIndices / 3;
    for (auto i = 0; i < numTriangles; ++i) {
      const auto index = &buffer[baseIndexIdx + (i * 3 * indexStride)];
      // Jolt uses uint32 for vertex indices
      triangles.push_back(convert(index, indexStride));
    }
  }

  return JPH::MeshShapeSettings{vertices, triangles};
}

ColliderLoader::result_type
ColliderLoader::operator()(const std::filesystem::path &p) const {
  try {
    const auto text = os::FileSystem::readText(p);
    if (!text) throw std::runtime_error{text.error()};
    const auto j = json::parse(*text);

    const auto type = j["type"].get<ColliderType>();
    JPH::ShapeSettings::ShapeResult shapeResult;

    const auto dir = p.parent_path();

    switch (type) {
    case ColliderType::Box: {
      const auto halfExtents = j["halfExtents"].get<glm::vec3>();
      JPH::BoxShapeSettings shapeSettings{to_Jolt(halfExtents)};
      shapeResult = shapeSettings.Create();
    } break;
    case ColliderType::Sphere: {
      JPH::SphereShapeSettings shapeSettings{j["radius"].get<float>()};
      shapeResult = shapeSettings.Create();
    } break;
    case ColliderType::Capsule: {
      JPH::CapsuleShapeSettings shapeSettings{
        j["halfHeight"],
        j["radius"],
        nullptr,
      };
      shapeResult = shapeSettings.Create();
    } break;

    case ColliderType::ConvexHull: {
      assert(false); // TODO ...
    } break;

    case ColliderType::TriangleMesh: {
      const auto relativePath = j.at("meta").get<std::string>();
      const auto mesh = readMesh(dir / relativePath);
      const auto shapeSettings = getShapeSettings(mesh);
      shapeResult = shapeSettings.Create();
    } break;
    }

    assert(shapeResult.IsValid());

    if (j.contains("offset")) {
      const auto &offset = j["offset"];
      const auto position = offset["position"].get<glm::vec3>();
      const auto q = offset["orientation"].get<glm::quat>();
      JPH::RotatedTranslatedShapeSettings offsetSettings{
        to_Jolt(position), to_Jolt(q), shapeResult.Get()};
      shapeResult = offsetSettings.Create();
    }
    if (j.contains("scale")) {
      const auto scale = j["scale"].get<glm::vec3>();
      JPH::ScaledShapeSettings settings{shapeResult.Get(), to_Jolt(scale)};
      shapeResult = settings.Create();
    }

    return std::make_shared<ColliderResource>(shapeResult.Get(),
                                              p.lexically_normal());
  } catch (const std::exception &e) {
    SPDLOG_WARN("{}: {}", os::FileSystem::relativeToRoot(p)->generic_string(),
                e.what());
    return nullptr;
  }
}
