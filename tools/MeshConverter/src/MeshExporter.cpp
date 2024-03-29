#include "MeshExporter.hpp"

#include "ai2glm.hpp"
#include "MakeSpan.hpp"
#include "Logger.hpp"
#include "assimp/Exceptional.h"

#include "ozz/animation/runtime/skeleton_utils.h"

#include "MaterialConverter.hpp"

#include "meshoptimizer.h"      // meshopt_simplify
#include "glm/gtc/type_ptr.hpp" // value_ptr

#include "math/json.hpp"
#include "rhi/json.hpp"
#include "renderer/jsonVertexFormat.hpp"

#include <fstream> // ofstream
#include <numeric> // accumulate

// https://github.com/assimp/assimp/blob/master/code/AssetLib/Obj/ObjExporter.cpp

namespace {

constexpr auto kMeshExtension = ".mesh";
constexpr auto kDataBufferName = "data.buffer";

constexpr auto kUninitializedJoint = -1;

[[nodiscard]] std::size_t countIndices(std::span<const aiFace> faces) {
  return std::accumulate(faces.cbegin(), faces.cend(), 0u,
                         [](const auto count, const aiFace &face) {
                           return count + face.mNumIndices;
                         });
}
[[nodiscard]] std::size_t countIndices(const aiMesh &mesh) {
  return countIndices(MAKE_SPAN(mesh, Faces));
}

void expand(AABB &master, const AABB &child) {
  if (child.min.x < master.min.x) master.min.x = child.min.x;
  if (child.min.y < master.min.y) master.min.y = child.min.y;
  if (child.min.z < master.min.z) master.min.z = child.min.z;

  if (child.max.x > master.max.x) master.max.x = child.max.x;
  if (child.max.y > master.max.y) master.max.y = child.max.y;
  if (child.max.z > master.max.z) master.max.z = child.max.z;
}
void expand(AABB &aabb, const glm::vec3 &v) {
  if (v.x < aabb.min.x) aabb.min.x = v.x;
  if (v.y < aabb.min.y) aabb.min.y = v.y;
  if (v.z < aabb.min.z) aabb.min.z = v.z;

  if (v.x > aabb.max.x) aabb.max.x = v.x;
  if (v.y > aabb.max.y) aabb.max.y = v.y;
  if (v.z > aabb.max.z) aabb.max.z = v.z;
}

void copy(const IndicesList &src, const offline::SubMesh::LOD &lod,
          IndicesList &dst) {
  dst.reserve(lod.numIndices);
  const auto begin = src.cbegin() + lod.indexOffset;
  const auto end = begin + lod.numIndices;
  std::copy(begin, end, std::back_inserter(dst));
}

[[nodiscard]] auto toByteBuffer(const auto &src, std::size_t stride) {
  ByteBuffer out(stride * src.size());
  auto data = out.data();
  for (auto value : src) {
    std::memcpy(data, &value, stride);
    data += stride;
  }
  return out;
}

[[nodiscard]] auto
makeInverseBindPose(const ozz::animation::Skeleton &skeleton,
                    const aiInverseBindPoseMap &inverseBindPoseMap) {
  assert(skeleton.num_joints() == inverseBindPoseMap.size());

  std::vector<glm::mat4> out;
  out.reserve(skeleton.num_joints());
  for (const auto *name : skeleton.joint_names()) {
    out.emplace_back(to_mat4(inverseBindPoseMap.find(aiString{name})->second));
  }
  return out;
}

} // namespace

namespace offline {

void to_json(nlohmann::ordered_json &j, const SubMesh::LOD &in) {
  j = nlohmann::ordered_json{
    {"indexOffset", in.indexOffset},
    {"numIndices", in.numIndices},
  };
}
void to_json(nlohmann::ordered_json &j, const SubMesh &in) {
  j = nlohmann::ordered_json{
    {"name", in.name},
    {"vertexOffset", in.vertexOffset},
    {"numVertices", in.numVertices},
    {"LOD", in.LODs},
    {"aabb", in.aabb},
  };
  if (in.material) {
    j.push_back({
      "material",
      std::format("materials/{0}/{0}.material", in.material->name),
    });
  }
}

void to_json(nlohmann::ordered_json &j, const Mesh &in) {
  j = nlohmann::ordered_json{
    {"buffer", kDataBufferName},
    {"meshes", in.subMeshes},
    {"aabb", in.aabb},
    {
      "vertices",
      {
        {"byteOffset", in.byteOffsets.vertices},
        {"count", in.vertices.getNumVertices()},
        {"stride", in.vertices.getStride()},
        {"attributes", in.vertices.info.getAttributes()},
      },
    },
    {
      "indices",
      {
        {"byteOffset", in.byteOffsets.indices},
        {"count", in.indices.size()},
        {"stride", in.indexStride},
      },
    },
  };
  if (!in.inverseBindPoseMap.empty()) {
    j["inverseBindPose"] = {
      {"byteOffset", in.byteOffsets.inverseBindPose},
      {"count", in.inverseBindPoseMap.size()},
    };
  }
}

} // namespace offline

//
// MeshExporter:
//

MeshExporter::MeshExporter(Flags flags) : m_flags{flags} {}

MeshExporter &MeshExporter::load(const aiScene *scene) {
  assert(scene);

  const auto meshes = MAKE_SPAN(*scene, Meshes);
  if (meshes.empty()) {
    throw DeadlyExportError{"No meshes were found."};
  }

  m_transforms = Transforms{scene->mRootNode->mTransformation};
  _prepareSkeleton(scene);
  _processMeshes(meshes, MAKE_SPAN(*scene, Materials));

  if (m_runtimeSkeleton) {
    m_animations = processAnimations(
      *m_runtimeSkeleton, MAKE_SPAN(*scene, Animations), m_transforms.root);
  }
  return *this;
}
MeshExporter &MeshExporter::save(const std::filesystem::path &p) {
  const auto dir = p.parent_path();
  std::filesystem::create_directory(dir);

  _writeDataBuffer(dir);
  if (!bool(m_flags & Flags::IgnoreMaterials)) {
    exportMaterials(m_mesh.subMeshes, dir / "materials");
  }
  _exportMeshMeta(p);

  if (m_runtimeSkeleton) {
    exportSkeleton(*m_runtimeSkeleton, dir);
    exportAnimations(m_animations, dir / "animations");
  }
  return *this;
}

//
// (private):
//

void MeshExporter::_prepareSkeleton(const aiScene *scene) {
  if (const auto rootBone = findRootBone(scene); rootBone) {
    LOG_EX(info, "Root bone: '{}'", rootBone->mName.C_Str());
    if (auto rawSkeleton = buildRawSkeleton(rootBone); rawSkeleton) {
      m_runtimeSkeleton = buildRuntimeSkeleton(*rawSkeleton);
      LOG_EX(info, "Skeleton has been built ({} joints).",
             m_runtimeSkeleton->num_joints());
    } else {
      LOG(error, rawSkeleton.error());
    }
  } else {
    LOG(info, "Skeleton not found.");
  }
}
void MeshExporter::_processMeshes(std::span<aiMesh *> meshes,
                                  std::span<aiMaterial *> materials) {
  assert(!meshes.empty());

  _findVertexFormat(meshes);
  for (const auto *mesh : meshes) {
    const auto &subMesh = _processMesh(*mesh, materials);
    expand(m_mesh.aabb, subMesh.aabb);
  }
  if (m_runtimeSkeleton) {
    for (const auto *name : m_runtimeSkeleton->joint_names()) {
      m_mesh.inverseBindPoseMap.try_emplace(aiString{name});
    }
  }

  assert(!m_mesh.subMeshes.empty());
  if (bool(m_flags & Flags::GenerateLODs)) {
    // (For safety) LOD generation likely increased the number of indices.
    m_mesh.updateIndexStride();
  }
  _buildBufferOffsets();
}
void MeshExporter::_findVertexFormat(std::span<aiMesh *> meshes) {
  using enum gfx::AttributeLocation;
  using enum rhi::VertexAttribute::Type;

  offline::Mesh::SizeInfo sizeInfo;

  VertexInfo::Builder builder;
  builder.add(Position, Float3);
  for (const auto *mesh : meshes) {
    if (mesh->GetNumColorChannels() > 0) builder.add(Color_0, Float4);
    if (mesh->HasNormals()) builder.add(Normal, Float3);
    if (const auto numTexCoords = mesh->GetNumUVChannels(); numTexCoords > 0) {
      builder.add(TexCoord_0, Float2);
      if (mesh->HasTangentsAndBitangents()) {
        builder.add(Tangent, Float3);
        builder.add(Bitangent, Float3);
      }
      if (numTexCoords > 1) builder.add(TexCoord_1, Float2);
    }
    if (mesh->HasBones()) {
      builder.add(Joints, Int4);
      builder.add(Weights, Float4);
    }

    sizeInfo.numVertices += mesh->mNumVertices;
    sizeInfo.numIndices += countIndices(*mesh);
  }
  m_mesh.vertices.info = builder.build();
  m_mesh.reserve(sizeInfo);
}
const offline::SubMesh &
MeshExporter::_processMesh(const aiMesh &mesh,
                           std::span<aiMaterial *> materials) {
  auto &subMesh =
    m_mesh.add(mesh.mName.C_Str(), {
                                     .numVertices = mesh.mNumVertices,
                                     .numIndices = countIndices(mesh),
                                   });
  if (!bool(m_flags & Flags::IgnoreMaterials)) {
    subMesh.material =
      convert(*materials[mesh.mMaterialIndex], mesh.mMaterialIndex);
  }
  if (mesh.HasBones()) {
    for (const auto *bone : MAKE_SPAN(mesh, Bones)) {
      m_mesh.inverseBindPoseMap.try_emplace(
        bone->mName, bone->mOffsetMatrix * m_transforms.inverseRoot);
    }
  }
  _fillVertexBuffer(mesh, subMesh);
  _fillIndexBuffer(mesh, subMesh);
  return subMesh;
}
void MeshExporter::_buildBufferOffsets() {
  const auto &lastSubMesh = m_mesh.subMeshes.back();
  assert((lastSubMesh.vertexOffset + lastSubMesh.numVertices) ==
         m_mesh.vertices.getNumVertices());

  const auto numIndices = m_mesh.indices.size();
  [[maybe_unused]] const auto &lastLOD = lastSubMesh.LODs.back();
  assert((lastLOD.indexOffset + lastLOD.numIndices) == numIndices);

  const auto vertexBufferByteSize = m_mesh.vertices.buffer.size();
  const auto indexBufferByteSize = m_mesh.indexStride * numIndices;
  m_mesh.byteOffsets = {
    .indices = vertexBufferByteSize,
    .inverseBindPose = vertexBufferByteSize + indexBufferByteSize,
  };
}

void MeshExporter::_fillVertexBuffer(const aiMesh &mesh,
                                     offline::SubMesh &subMesh) {
  const auto &vertices = m_mesh.vertices;
  auto *currentVertex = m_mesh.vertexAt(subMesh);

  const auto copySlice = [&vertices,
                          &currentVertex](gfx::AttributeLocation location,
                                          const auto &value, std::size_t size) {
    const auto &attribute = vertices.info.getAttribute(location);
    assert(size == rhi::getSize(attribute.type));
    std::memcpy(currentVertex + attribute.offset, glm::value_ptr(value), size);
  };

  using enum gfx::AttributeLocation;

  for (auto i = 0u; i < mesh.mNumVertices; ++i) {
    const auto position = to_vec3(m_transforms.root * mesh.mVertices[i]);
    expand(subMesh.aabb, position);

    copySlice(Position, position, sizeof(glm::vec3));
    if (mesh.GetNumColorChannels() > 0) {
      copySlice(Color_0, to_vec4(mesh.mColors[0][i]), sizeof(glm::vec4));
    }
    if (mesh.HasNormals()) {
      copySlice(
        Normal,
        to_vec3((m_transforms.normalMatrix * mesh.mNormals[i]).Normalize()),
        sizeof(glm::vec3));
    }
    if (mesh.mTextureCoords[0]) {
      copySlice(TexCoord_0, to_vec2(mesh.mTextureCoords[0][i]),
                sizeof(glm::vec2));
      if (mesh.HasTangentsAndBitangents()) {
        copySlice(
          Tangent,
          to_vec3((m_transforms.normalMatrix * mesh.mTangents[i]).Normalize()),
          sizeof(glm::vec3));
        copySlice(
          Bitangent,
          to_vec3(
            (m_transforms.normalMatrix * mesh.mBitangents[i]).Normalize()),
          sizeof(glm::vec3));
      }
    }
    if (mesh.mTextureCoords[1]) {
      copySlice(TexCoord_1, to_vec2(mesh.mTextureCoords[1][i]),
                sizeof(glm::vec2));
    }

    if (mesh.HasBones()) {
      copySlice(Joints, glm::ivec4{kUninitializedJoint}, sizeof(glm::ivec4));
      copySlice(Weights, glm::vec4{0.0f}, sizeof(glm::vec4));
    }

    currentVertex += vertices.getStride();
  }

  if (mesh.HasBones()) {
    const auto &jointsAttrib = vertices.info.getAttribute(Joints);
    const auto &weightAttrib = vertices.info.getAttribute(Weights);

    for (const auto *bone : MAKE_SPAN(mesh, Bones)) {
      const auto jointIndex =
        ozz::animation::FindJoint(*m_runtimeSkeleton, bone->mName.C_Str());
      assert(jointIndex != -1);

      for (const auto &weight : MAKE_SPAN(*bone, Weights)) {
        auto *vertex = m_mesh.vertexAt(subMesh, weight.mVertexId);

        auto *joints = std::bit_cast<int32_t *>(vertex + jointsAttrib.offset);
        auto *weights = std::bit_cast<float *>(vertex + weightAttrib.offset);

        constexpr auto kNumWeights = 4;
        for (auto i = 0u; i < kNumWeights; ++i) {
          if (joints[i] == kUninitializedJoint) {
            assert(weights[i] == 0.0f);
            *(joints + i) = jointIndex;
            *(weights + i) = weight.mWeight;
            break;
          }
        }
      }
    }
  }
}
void MeshExporter::_fillIndexBuffer(const aiMesh &mesh,
                                    offline::SubMesh &subMesh) {
  for (const auto &face : MAKE_SPAN(mesh, Faces)) {
    std::ranges::copy(MAKE_SPAN(face, Indices),
                      std::back_inserter(m_mesh.indices));
  }
  if (bool(m_flags & Flags::GenerateLODs)) _generateLODs(subMesh, 3);
}

void MeshExporter::_generateLODs(offline::SubMesh &subMesh,
                                 std::size_t numLevels) {
  const auto &baseLevel = subMesh.LODs.front();

  // https://github.com/zeux/meshoptimizer/blob/577c585eeb477e18902bd7f3d66bf33d89216cef/demo/main.cpp#L536

  std::vector<IndicesList> LODs(numLevels);
  copy(m_mesh.indices, baseLevel, LODs[0]);

  const auto vertexStride = m_mesh.vertices.info.getStride();
  const auto *firstVertex = std::bit_cast<float *>(m_mesh.vertexAt(subMesh));

  for (auto i = 1u; i < numLevels; ++i) {
    auto &currentLevel = LODs[i];

    const auto threshold = powf(0.7f, static_cast<float>(i));
    auto targetIndexCount =
      std::size_t(baseLevel.numIndices * threshold) / 3 * 3;
    constexpr auto kTargetError = 1e-2f;

    const auto &src = LODs[i - 1];
    if (src.size() < targetIndexCount) targetIndexCount = src.size();

    currentLevel.resize(src.size());
    currentLevel.resize(meshopt_simplify(
      &currentLevel[0], &src[0], src.size(), firstVertex, subMesh.numVertices,
      vertexStride, targetIndexCount, kTargetError));

    if (currentLevel.size() != src.size()) {
      subMesh.addLOD(m_mesh.indices, currentLevel);
    }
  }
}

bool MeshExporter::_writeDataBuffer(const std::filesystem::path &dir) const {
  std::ofstream f{dir / kDataBufferName, std::ios::binary};
  if (!f.is_open()) return false;

  const auto write = [&f](const auto &container) {
    const auto elementSize = sizeof(decltype(container.back()));
    f.write(std::bit_cast<const char *>(container.data()),
            container.size() * elementSize);
  };

  write(m_mesh.vertices.buffer);
  write(toByteBuffer(m_mesh.indices, m_mesh.indexStride));
  if (m_runtimeSkeleton) {
    write(makeInverseBindPose(*m_runtimeSkeleton, m_mesh.inverseBindPoseMap));
  }
  return true;
}
bool MeshExporter::_exportMeshMeta(const std::filesystem::path &p) const {
  if (std::ofstream f{p}; f.is_open()) {
    nlohmann::ordered_json j{m_mesh};
    f << std::setw(2) << j.front() << std::endl;
    return true;
  }
  return false;
}
