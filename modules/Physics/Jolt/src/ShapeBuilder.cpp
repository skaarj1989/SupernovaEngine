#include "physics/ShapeBuilder.hpp"
#include "TypeTraits.hpp"
#include "os/FileSystem.hpp"

#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/StaticCompoundShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"

#include "physics/Conversion.hpp"

#include "nlohmann/json.hpp"

using namespace nlohmann;

static void from_json(const json &j, TriangleMesh::Info::Vertex &out) {
  j.at("byteOffset").get_to(out.byteOffset);
  j.at("count").get_to(out.count);
  j.at("stride").get_to(out.stride);
}
static void from_json(const json &j, TriangleMesh::Info::Index &out) {
  j.at("byteOffset").get_to(out.byteOffset);
  j.at("count").get_to(out.count);
  j.at("stride").get_to(out.stride);
}
static void from_json(const json &j, TriangleMesh::Info::SubMesh::LOD &out) {
  j.at("indexOffset").get_to(out.indexOffset);
  j.at("numIndices").get_to(out.numIndices);
}
static void from_json(const json &j, TriangleMesh::Info::SubMesh &out) {
  j.at("vertexOffset").get_to(out.vertexOffset);
  j.at("numVertices").get_to(out.numVertices);
  j.at("LOD").get_to(out.LODs);
}
static void from_json(const json &j, TriangleMesh::Info &out) {
  j.at("vertices").get_to(out.vertices);
  j.at("indices").get_to(out.indices);
  j.at("meshes").get_to(out.subMeshes);
}

std::expected<TriangleMesh, std::string>
readTriangleMesh(const std::filesystem::path &p) {
  const auto text = os::FileSystem::readText(p);
  if (!text) {
    return std::unexpected{text.error()};
  }
  try {
    const auto j = json::parse(*text);

    const auto dir = p.parent_path();
    const auto relativePath = j.at("buffer").get<std::string>();
    auto buffer = os::FileSystem::readBuffer(dir / relativePath);
    if (!buffer) throw std::runtime_error{buffer.error()};

    return TriangleMesh{
      .info = j,
      .buffer = std::move(buffer->data),
    };
  } catch (const std::exception &e) {
    return std::unexpected{e.what()};
  }
}

namespace {

template <typename T>
  requires is_any_v<T, uint16_t, uint32_t>
[[nodiscard]] JPH::IndexedTriangle
makeIndexedTriangle(const std::byte *buffer) {
  return {
    *std::bit_cast<T *>(buffer),
    *std::bit_cast<T *>(buffer + sizeof(T)),
    *std::bit_cast<T *>(buffer + (2 * sizeof(T))),
  };
}

[[nodiscard]] auto makeIndexedTriangle(const std::byte *buffer,
                                       uint32_t stride) {
  switch (stride) {
  case 2:
    return makeIndexedTriangle<uint16_t>(buffer);
  case 4:
    return makeIndexedTriangle<uint32_t>(buffer);
  default:
    assert(false);
  }
  return JPH::IndexedTriangle{};
}

template <typename T, typename Func>
[[nodiscard]] auto convert(std::span<const T> in, Func func) {
  JPH::Array<JPH::Vec3> out;
  out.reserve(in.size());
  std::ranges::transform(in, std::back_inserter(out), func);
  return out;
}
[[nodiscard]] auto convert(std::span<const JPH::Float3> in) {
  return convert(in, [](const auto &v) { return JPH::Vec3{v}; });
}
[[nodiscard]] auto convert(std::span<const glm::vec3> in) {
  return convert(in, [](const auto &v) { return to_Jolt(v); });
}

struct RawMesh {
  JPH::VertexList vertices;
  JPH::IndexedTriangleList triangles;
};
[[nodiscard]] auto toMesh(const RawMesh &in) {
  return JPH::MeshShapeSettings{in.vertices, in.triangles}.Create();
}
[[nodiscard]] auto toConvexHull(const RawMesh &in) {
  return JPH::ConvexHullShapeSettings{convert(in.vertices)}.Create();
}

[[nodiscard]] std::vector<RawMesh> convert(const TriangleMesh &in) {
  const auto &[meta, buffer] = in;
  if (meta.subMeshes.empty()) {
    return {};
  }

  static_assert(
    std::is_same_v<decltype(glm::vec3::x), decltype(JPH::Float3::x)> &&
    std::is_same_v<decltype(glm::vec3::y), decltype(JPH::Float3::y)> &&
    std::is_same_v<decltype(glm::vec3::z), decltype(JPH::Float3::z)>);

  const auto vertexStride = meta.vertices.stride;
  const auto indexStride = meta.indices.stride;

  std::vector<RawMesh> out;
  out.reserve(meta.subMeshes.size());
  for (const auto &subMesh : meta.subMeshes) {
    auto &[vertices, triangles] = out.emplace_back();
    vertices.reserve(subMesh.numVertices);

    const auto baseVertexIdx =
      meta.vertices.byteOffset + (subMesh.vertexOffset * vertexStride);
    for (auto i = 0u; i < subMesh.numVertices; ++i) {
      const auto vertex = &buffer[baseVertexIdx + (i * vertexStride)];
      vertices.push_back(*std::bit_cast<const JPH::Float3 *>(vertex));
    }

    const auto &LOD = subMesh.LODs.back();
    const auto numTriangles = LOD.numIndices / 3;
    triangles.reserve(numTriangles);

    const auto baseIndexIdx =
      meta.indices.byteOffset + (LOD.indexOffset * indexStride);
    for (auto i = 0u; i < numTriangles; ++i) {
      const auto index = &buffer[baseIndexIdx + (i * 3 * indexStride)];
      // Jolt uses uint32 for vertex indices
      triangles.push_back(makeIndexedTriangle(index, indexStride));
    }
  }
  return out;
}

template <typename Func>
[[nodiscard]] auto convert(const TriangleMesh &mesh, Func func) {
  std::vector<UncookedShape> out;
  out.reserve(mesh.info.subMeshes.size());
  std::ranges::transform(convert(mesh), std::back_inserter(out), func);
  return out;
}

} // namespace

//
// ShapeBuilder  class:
//

UncookedShape ShapeBuilder::makeSphere(float radius) {
  return JPH::SphereShapeSettings{radius}.Create();
}
UncookedShape ShapeBuilder::makeBox(const glm::vec3 &halfExtent) {
  return JPH::BoxShapeSettings{to_Jolt(halfExtent)}.Create();
}
UncookedShape ShapeBuilder::makeCapsule(float halfHeight, float radius) {
  return JPH::CapsuleShapeSettings{halfHeight, radius, nullptr}.Create();
}
UncookedShape ShapeBuilder::makeConvexHull(const std::vector<glm::vec3> &in) {
  return JPH::ConvexHullShapeSettings{convert(in)}.Create();
}

UncookedShape
ShapeBuilder::makeCompound(std::span<const ShapeBuilder::Entry> in) {
  JPH::StaticCompoundShapeSettings compound;
  for (const auto &[s, xf] : in) {
    compound.AddShape(xf ? to_Jolt(xf->position) : JPH::Vec3::sZero(),
                      xf ? to_Jolt(xf->orientation) : JPH::QuatArg::sIdentity(),
                      s.Get());
  }
  return compound.Create();
}
UncookedShape ShapeBuilder::makeCompound(std::span<const UncookedShape> in) {
  JPH::StaticCompoundShapeSettings compound;
  for (const auto &s : in) {
    compound.AddShape(JPH::Vec3::sZero(), JPH::QuatArg::sIdentity(), s.Get());
  }
  return compound.Create();
}

std::vector<UncookedShape> ShapeBuilder::toMesh(const TriangleMesh &mesh) {
  return convert(mesh, [](const auto &in) { return ::toMesh(in); });
}
std::vector<UncookedShape>
ShapeBuilder::toConvexHull(const TriangleMesh &mesh) {
  return convert(mesh, [](const auto &in) { return ::toConvexHull(in); });
}

ShapeBuilder &ShapeBuilder::set(UncookedShape in, std::optional<Transform> xf) {
  return reset().add(std::move(in), std::move(xf));
}
ShapeBuilder &ShapeBuilder::ShapeBuilder::reset() {
  m_entries.clear();
  return *this;
}

ShapeBuilder &ShapeBuilder::add(UncookedShape in, std::optional<Transform> xf) {
  if (in.IsValid()) {
    m_entries.emplace_back(std::move(in), std::move(xf));
  }
  return *this;
}

ShapeBuilder &ShapeBuilder::scale(const glm::vec3 &v) {
  if (size() > 0) {
    _flatten();
    auto &[settings, xf] = m_entries.front();
    assert(settings.IsValid());
    settings = JPH::ScaledShapeSettings{settings.Get(), to_Jolt(v)}.Create();
    xf = std::nullopt;
  }
  return *this;
}
ShapeBuilder &ShapeBuilder::rotateTranslate(const glm::vec3 &v,
                                            const glm::quat &q) {
  if (size() > 0) {
    _flatten();
    auto &[settings, xf] = m_entries.front();
    assert(settings.IsValid());
    settings = JPH::RotatedTranslatedShapeSettings{to_Jolt(v), to_Jolt(q),
                                                   settings.Get()}
                 .Create();
    xf = std::nullopt;
  }
  return *this;
}

UncookedShape ShapeBuilder::build() const { return _flatten(m_entries); }

//
// (private):
//

UncookedShape ShapeBuilder::_flatten(std::span<const Entry> entries) {
  return entries.empty() ? UncookedShape{} : makeCompound(entries);
}
bool ShapeBuilder::_flatten() {
  if (auto s = _flatten(m_entries); s.IsValid()) {
    set(std::move(s));
    return true;
  } else {
    reset();
    return false;
  }
}
