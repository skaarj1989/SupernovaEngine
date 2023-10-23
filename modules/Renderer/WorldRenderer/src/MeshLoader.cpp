#include "renderer/MeshLoader.hpp"
#include "rhi/RenderDevice.hpp"
#include "os/FileSystem.hpp"
#include "rhi/json.hpp"
#include "renderer/jsonVertexFormat.hpp"
#include "renderer/jsonMaterial.hpp"
#include "spdlog/spdlog.h"

using namespace nlohmann;

namespace gfx {

namespace {

using AttributeMap = std::map<AttributeLocation, rhi::VertexAttribute>;

[[nodiscard]] auto buildVertexFormat(const AttributeMap &attributes) {
  VertexFormat::Builder builder{};
  for (const auto &[location, attribute] : attributes)
    builder.setAttribute(location, attribute);

  return builder.build();
}

struct BufferMeta {
  struct Info {
    std::size_t byteOffset{0};
    std::size_t count{0};
    std::size_t stride{0};
    [[nodiscard]] constexpr auto dataSize() const { return stride * count; }

    [[nodiscard]] rhi::Buffer createStagingBuffer(rhi::RenderDevice &rd,
                                                  const std::byte *data) const {
      return rd.createStagingBuffer(dataSize(), data + byteOffset);
    }
  };

  struct Vertices : Info {
    AttributeMap attributes;
  };
  struct InverseBindPose : Info {
    [[nodiscard]] auto get(const std::byte *data) const {
      gfx::Joints m(count);
      std::memcpy(m.data(), data + byteOffset, dataSize());
      return m;
    }
  };

  Vertices vertices;
  std::optional<Info> indices;
  std::optional<InverseBindPose> inverseBindPose;
};

void from_json(const json &j, BufferMeta::Info &out) {
  j.at("byteOffset").get_to(out.byteOffset);
  j.at("count").get_to(out.count);
  out.stride = j.value("stride", 0);
}
void from_json(const json &j, BufferMeta::Vertices &out) {
  from_json(j, static_cast<BufferMeta::Info &>(out));
  j.at("attributes").get_to(out.attributes);
}
void from_json(const json &j, BufferMeta::InverseBindPose &out) {
  from_json(j, static_cast<BufferMeta::Info &>(out));
  out.stride = sizeof(Joints::value_type);
}

struct MeshMeta {
  std::filesystem::path bufferPath; // Relative to mesh root file.

  struct SubMesh {
    std::string name;

    uint32_t vertexOffset{0};
    uint32_t numVertices{0};
    struct LOD {
      uint32_t indexOffset{0};
      uint32_t numIndices{0};
    };
    std::vector<LOD> LODs;
    AABB aabb;

    std::filesystem::path materialPath; // Relative to mesh root file.
  };
  std::vector<SubMesh> meshes;
  AABB aabb;

  BufferMeta bufferMeta;
};

void from_json(const json &j, MeshMeta::SubMesh::LOD &out) {
  j.at("indexOffset").get_to(out.indexOffset);
  j.at("numIndices").get_to(out.numIndices);
}
void from_json(const json &j, MeshMeta::SubMesh &out) {
  j.at("name").get_to(out.name);
  j.at("vertexOffset").get_to(out.vertexOffset);
  j.at("numVertices").get_to(out.numVertices);
  j.at("LOD").get_to(out.LODs);
  j.at("aabb").get_to(out.aabb);
  out.materialPath = j.value("material", "");
}

void from_json(const json &j, MeshMeta &out) {
  j.at("buffer").get_to(out.bufferPath);
  j.at("meshes").get_to(out.meshes);
  j.at("aabb").get_to(out.aabb);
  j.at("vertices").get_to(out.bufferMeta.vertices);
  if (j.contains("indices")) {
    out.bufferMeta.indices = j.at("indices").get<BufferMeta::Info>();
  }
  if (j.contains("inverseBindPose")) {
    out.bufferMeta.inverseBindPose =
      j.at("inverseBindPose").get<BufferMeta::InverseBindPose>();
  }
}

} // namespace

MeshLoader::result_type MeshLoader::operator()(const std::filesystem::path &p,
                                               MaterialManager &materialManager,
                                               rhi::RenderDevice &rd) const {
  try {
    const auto text = os::FileSystem::readText(p);
    if (!text) throw std::runtime_error{text.error()};
    const MeshMeta meshMeta = json::parse(*text);

    const auto [data, bufferSize] =
      *os::FileSystem::readBuffer(p.parent_path() / meshMeta.bufferPath);
    const auto &[vertices, indices, inverseBindPose] = meshMeta.bufferMeta;

    auto vertexFormat = buildVertexFormat(vertices.attributes);
    if (vertexFormat->getStride() != vertices.stride)
      throw std::runtime_error{"Vertex stride mismatch!"};

    auto vertexBuffer = rd.createVertexBuffer(vertices.stride, vertices.count);
    auto stagingVertexBuffer = vertices.createStagingBuffer(rd, data.get());

    rhi::IndexBuffer indexBuffer;
    rhi::Buffer stagingIndexBuffer;

    if (indices) {
      indexBuffer =
        rd.createIndexBuffer(rhi::IndexType(indices->stride), indices->count);
      stagingIndexBuffer = indices->createStagingBuffer(rd, data.get());
    }

    rd.execute([&](rhi::CommandBuffer &cb) {
      cb.copyBuffer(stagingVertexBuffer, vertexBuffer,
                    {.size = vertices.dataSize()});
      if (stagingIndexBuffer) {
        cb.copyBuffer(stagingIndexBuffer, indexBuffer,
                      {.size = indices->dataSize()});
      }
    });

    // ---

    Mesh::Builder builder{};
    builder.setVertexFormat(vertexFormat)
      .setVertexBuffer(
        rhi::makeShared<rhi::VertexBuffer>(rd, std::move(vertexBuffer)))
      .setIndexBuffer(
        rhi::makeShared<rhi::IndexBuffer>(rd, std::move(indexBuffer)))
      .setAABB(meshMeta.aabb);

    if (inverseBindPose) {
      builder.setInverseBindPose(inverseBindPose->get(data.get()));
    }

    const auto dir = p.parent_path();

    for (const auto &subMesh : meshMeta.meshes) {
      MaterialResourceHandle material;
      if (!subMesh.materialPath.empty()) {
        material = materialManager.load(dir / subMesh.materialPath);
      }
      builder.beginSubMesh(subMesh.vertexOffset, subMesh.numVertices,
                           material.handle(), subMesh.aabb);
      for (const auto &lod : subMesh.LODs)
        builder.addLOD(lod.indexOffset, lod.numIndices);
    }

    return std::make_shared<MeshResource>(builder.build(), p);
  } catch (const std::exception &e) {
    SPDLOG_WARN("{}: {}", os::FileSystem::relativeToRoot(p)->generic_string(),
                e.what());
    return nullptr;
  }
}

MeshLoader::result_type MeshLoader::operator()(const std::string_view name,
                                               Mesh &&mesh) const {
  return std::make_shared<MeshResource>(std::move(mesh), name);
}

} // namespace gfx
