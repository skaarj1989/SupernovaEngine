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

  const auto numVertices = planeVertices.size() +
                           subdividedPlane.vertices.size() +
                           cubeVertices.size() + sphere.vertices.size();
  const auto vertexDataSize = sizeof(Vertex1p1n1st) * numVertices;
  auto stagingVertexBuffer = rd.createStagingBuffer(vertexDataSize);
  auto vertices = static_cast<Vertex1p1n1st *>(stagingVertexBuffer.map());

  auto vertexOffset = 0;
  const auto copyVertices = [&](const auto &src) {
    memcpy(vertices + vertexOffset, src.data(),
           src.size() * sizeof(Vertex1p1n1st));
    vertexOffset += src.size();
    return vertexOffset;
  };

  const auto subdividedPlaneVertexOffset = copyVertices(planeVertices);
  const auto cubeVertexOffset = copyVertices(subdividedPlane.vertices);
  const auto sphereVertexOffset = copyVertices(cubeVertices);
  copyVertices(sphere.vertices);

  stagingVertexBuffer.unmap();

  // -- Fill staging IndexBuffer:

  const auto numIndices =
    subdividedPlane.indices.size() + sphere.indices.size();
  const auto indexDataSize = sizeof(uint16_t) * numIndices;
  auto stagingIndexBuffer = rd.createStagingBuffer(indexDataSize);
  auto indices = static_cast<uint16_t *>(stagingIndexBuffer.map());

  auto indexOffset = 0;
  const auto copyIndices = [&](const auto &src) {
    memcpy(indices + indexOffset, src.data(), src.size() * sizeof(uint16_t));
    indexOffset += src.size();
    return indexOffset;
  };

  const auto sphereIndexOffset = copyIndices(subdividedPlane.indices);
  copyIndices(sphere.indices);

  stagingIndexBuffer.unmap();

  // -- Copy data to GPU:

  auto vertexBuffer = std::make_shared<rhi::VertexBuffer>(
    rd.createVertexBuffer(sizeof(Vertex1p1n1st), numVertices));
  auto indexBuffer = std::make_shared<rhi::IndexBuffer>(
    rd.createIndexBuffer(rhi::IndexType::UInt16, numIndices));

  rd.execute([&](auto &cb) {
    cb.copyBuffer(stagingVertexBuffer, *vertexBuffer, {.size = vertexDataSize})
      .copyBuffer(stagingIndexBuffer, *indexBuffer, {.size = indexDataSize});
  });

  // ---

  auto vertexFormat = Vertex1p1n1st::getVertexFormat();

  auto defaultMaterial = m_materialManager[MaterialManager::kDefault].handle();

  import(BasicShapes::Plane.data(),
         Mesh::Builder{}
           .setVertexFormat(vertexFormat)
           .setVertexBuffer(vertexBuffer)
           .beginSubMesh(0, planeVertices.size(), defaultMaterial,
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
                         subdividedPlane.vertices.size(), defaultMaterial,
                         {
                           .min = {-kPlaneSize, 0.0f, -kPlaneSize},
                           .max = {kPlaneSize, 0.0f, kPlaneSize},
                         })
           .addLOD(0, subdividedPlane.indices.size())
           .build());

  import(BasicShapes::Cube.data(),
         Mesh::Builder{}
           .setVertexFormat(vertexFormat)
           .setVertexBuffer(vertexBuffer)
           .beginSubMesh(cubeVertexOffset, cubeVertices.size(), defaultMaterial,
                         {
                           .min = glm::vec3{-kCubeSize},
                           .max = glm::vec3{kCubeSize},
                         })
           .build());

  import(BasicShapes::Sphere.data(),
         Mesh::Builder{}
           .setVertexFormat(vertexFormat)
           .setVertexBuffer(vertexBuffer)
           .setIndexBuffer(indexBuffer)
           .beginSubMesh(sphereVertexOffset, uint32_t(sphere.vertices.size()),
                         defaultMaterial,
                         {
                           .min = glm::vec3{-kSphereRadius},
                           .max = glm::vec3{kSphereRadius},
                         })
           .addLOD(sphereIndexOffset, uint32_t(sphere.indices.size()))
           .build());
}

bool MeshManager::isBuiltIn(const std::filesystem::path &p) const {
  return isBuiltIn(entt::hashed_string{p.string().c_str()});
}
bool MeshManager::isBuiltIn(uint32_t id) const {
  static const auto kBuiltInMeshIds = std::array{
    BasicShapes::Plane,
    BasicShapes::SubdividedPlane,
    BasicShapes::Cube,
    BasicShapes::Sphere,
  };
  return std::ranges::any_of(kBuiltInMeshIds,
                             [id](auto _id) { return id == _id; });
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
