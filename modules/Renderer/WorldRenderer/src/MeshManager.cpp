#include "renderer/MeshManager.hpp"
#include "rhi/RenderDevice.hpp"
#include "BasicShapes.hpp"
#include "LoaderHelper.hpp"

namespace gfx {

const entt::hashed_string MeshManager::BasicShapes::Plane = "BasicShapes:Plane";
const entt::hashed_string MeshManager::BasicShapes::SubdividedPlane =
  "BasicShapes:SubdividedPlane";
const entt::hashed_string MeshManager::BasicShapes::Cube = "BasicShapes:Cube";
const entt::hashed_string MeshManager::BasicShapes::Sphere =
  "BasicShapes:Sphere";

MeshManager::MeshManager(rhi::RenderDevice &rd,
                         MaterialManager &materialManager)
    : m_renderDevice{rd}, m_materialManager{materialManager} {
  constexpr auto kPlaneSize = 10;
  const auto planeVertices = buildPlane(kPlaneSize);

  const auto subdividedPlane =
    buildSubdividedPlane<kPlaneSize * 2 + 1, kPlaneSize * 2 + 1>();

  constexpr auto kCubeSize = 0.5f;
  const auto cubeVertices = buildCube(kCubeSize);

  constexpr auto kSphereRadius = 0.5f;
  const auto sphere = buildSphere<>(kSphereRadius);

  // -- Fill staging VertexBuffer:

  using VertexT = Vertex1p1n1st;

  const auto numVertices = planeVertices.size() +
                           subdividedPlane.vertices.size() +
                           cubeVertices.size() + sphere.vertices.size();
  const auto vertexDataSize = sizeof(VertexT) * numVertices;
  auto stagingVertexBuffer = rd.createStagingBuffer(vertexDataSize);
  auto *vertices = static_cast<VertexT *>(stagingVertexBuffer.map());

  uint32_t vertexOffset{0};
  const auto copyVertices = [&](const std::span<const VertexT> src) {
    memcpy(vertices + vertexOffset, src.data(), src.size() * sizeof(VertexT));
    vertexOffset += static_cast<uint32_t>(src.size());
    return vertexOffset;
  };

  const auto subdividedPlaneVertexOffset = copyVertices(planeVertices);
  const auto cubeVertexOffset = copyVertices(subdividedPlane.vertices);
  const auto sphereVertexOffset = copyVertices(cubeVertices);
  copyVertices(sphere.vertices);

  stagingVertexBuffer.unmap();

  // -- Fill staging IndexBuffer:

  using IndexT = uint16_t;

  const auto numIndices =
    subdividedPlane.indices.size() + sphere.indices.size();
  const auto indexDataSize = sizeof(IndexT) * numIndices;
  auto stagingIndexBuffer = rd.createStagingBuffer(indexDataSize);
  auto *indices = static_cast<IndexT *>(stagingIndexBuffer.map());

  uint32_t indexOffset{0};
  const auto copyIndices = [&](const auto &src) {
    memcpy(indices + indexOffset, src.data(), src.size() * sizeof(IndexT));
    indexOffset += static_cast<uint32_t>(src.size());
    return indexOffset;
  };

  const auto sphereIndexOffset = copyIndices(subdividedPlane.indices);
  copyIndices(sphere.indices);

  stagingIndexBuffer.unmap();

  // -- Copy data to GPU:

  auto vertexBuffer = std::make_shared<rhi::VertexBuffer>(
    rd.createVertexBuffer(sizeof(VertexT), numVertices));
  auto indexBuffer = std::make_shared<rhi::IndexBuffer>(
    rd.createIndexBuffer(rhi::IndexType::UInt16, numIndices));

  rd.execute([&](auto &cb) {
    cb.copyBuffer(stagingVertexBuffer, *vertexBuffer, {.size = vertexDataSize})
      .copyBuffer(stagingIndexBuffer, *indexBuffer, {.size = indexDataSize});
  });

  // ---

  auto vertexFormat = VertexT::getVertexFormat();

  auto defaultMaterial = m_materialManager[MaterialManager::kDefault].handle();

  import(BasicShapes::Plane.data(),
         Mesh::Builder{}
           .setVertexFormat(vertexFormat)
           .setVertexBuffer(vertexBuffer)
           .beginSubMesh(0, static_cast<uint32_t>(planeVertices.size()),
                         defaultMaterial,
                         {
                           .min = {-kPlaneSize, 0.0f, -kPlaneSize},
                           .max = {kPlaneSize, 0.0f, kPlaneSize},
                         })
           .build());

  import(BasicShapes::SubdividedPlane.data(),
         Mesh::Builder{}
           .setVertexFormat(vertexFormat)
           .setVertexBuffer(vertexBuffer)
           .setIndexBuffer(indexBuffer)
           .setTopology(rhi::PrimitiveTopology::TriangleStrip)
           .beginSubMesh(subdividedPlaneVertexOffset,
                         static_cast<uint32_t>(subdividedPlane.vertices.size()),
                         defaultMaterial,
                         {
                           .min = {-kPlaneSize, 0.0f, -kPlaneSize},
                           .max = {kPlaneSize, 0.0f, kPlaneSize},
                         })
           .addLOD(0, static_cast<uint32_t>(subdividedPlane.indices.size()))
           .build());

  import(BasicShapes::Cube.data(),
         Mesh::Builder{}
           .setVertexFormat(vertexFormat)
           .setVertexBuffer(vertexBuffer)
           .beginSubMesh(cubeVertexOffset,
                         static_cast<uint32_t>(cubeVertices.size()),
                         defaultMaterial,
                         {
                           .min = glm::vec3{-kCubeSize},
                           .max = glm::vec3{kCubeSize},
                         })
           .build());

  import(
    BasicShapes::Sphere.data(),
    Mesh::Builder{}
      .setVertexFormat(vertexFormat)
      .setVertexBuffer(vertexBuffer)
      .setIndexBuffer(indexBuffer)
      .beginSubMesh(sphereVertexOffset,
                    static_cast<uint32_t>(sphere.vertices.size()),
                    defaultMaterial,
                    {
                      .min = glm::vec3{-kSphereRadius},
                      .max = glm::vec3{kSphereRadius},
                    })
      .addLOD(sphereIndexOffset, static_cast<uint32_t>(sphere.indices.size()))
      .build());
}

bool MeshManager::isBuiltIn(const std::filesystem::path &p) const {
  return isBuiltIn(entt::hashed_string{p.string().c_str()});
}
bool MeshManager::isBuiltIn(const uint32_t id) const {
  static const auto kBuiltInMeshIds = std::array{
    BasicShapes::Plane,
    BasicShapes::SubdividedPlane,
    BasicShapes::Cube,
    BasicShapes::Sphere,
  };
  return std::ranges::any_of(
    kBuiltInMeshIds, [id](const auto builtInId) { return id == builtInId; });
}

MeshResourceHandle MeshManager::load(const std::filesystem::path &p) {
  return ::load(*this, p, isBuiltIn(p) ? LoadMode::BuiltIn : LoadMode::External,
                m_materialManager, m_renderDevice);
}
void MeshManager::import(const std::string_view name, Mesh &&mesh) {
  MeshCache::load(entt::hashed_string{name.data()}.value(), name,
                  std::move(mesh));
}

void MeshManager::clear() {
  const auto lastNotRemoved = std::remove_if(
    begin(), end(), [this](const auto it) { return !isBuiltIn(it.first); });
  erase(lastNotRemoved, end());
}

} // namespace gfx
