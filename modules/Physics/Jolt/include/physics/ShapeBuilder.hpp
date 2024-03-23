#pragma once

#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"

#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

#include <expected>
#include <filesystem>
#include <memory>
#include <span>

struct TriangleMesh {
  struct Info {
    struct Vertex {
      uint32_t byteOffset;
      uint32_t count;
      uint32_t stride;
    };
    struct Index {
      uint32_t byteOffset;
      uint32_t count;
      uint32_t stride;
    };
    Vertex vertices;
    Index indices;

    struct SubMesh {
      uint32_t vertexOffset;
      uint32_t numVertices;
      struct LOD {
        uint32_t indexOffset;
        uint32_t numIndices;
      };
      std::vector<LOD> LODs;
    };
    std::vector<SubMesh> subMeshes;
  };
  Info info;
  std::unique_ptr<std::byte[]> buffer;
};

[[nodiscard]] std::expected<TriangleMesh, std::string>
readTriangleMesh(const std::filesystem::path &);

using UncookedShape = JPH::ShapeSettings::ShapeResult;

class ShapeBuilder {
public:
  struct Transform {
    glm::vec3 position{0.0f};
    glm::quat orientation{glm::identity<glm::quat>()};

    [[nodiscard]] bool isIdentity() const {
      return position == glm::vec3{0.0f} &&
             orientation == glm::identity<glm::quat>();
    }
  };
  using Entry = std::pair<UncookedShape, std::optional<Transform>>;

  [[nodiscard]] static UncookedShape makeSphere(float radius);
  [[nodiscard]] static UncookedShape makeBox(const glm::vec3 &halfExtent);
  [[nodiscard]] static UncookedShape makeCapsule(float halfHeight,
                                                 float radius);
  [[nodiscard]] static UncookedShape
  makeConvexHull(const std::vector<glm::vec3> &);

  [[nodiscard]] static UncookedShape makeCompound(std::span<const Entry>);
  [[nodiscard]] static UncookedShape
    makeCompound(std::span<const UncookedShape>);

  [[nodiscard]] static std::vector<UncookedShape> toMesh(const TriangleMesh &);
  [[nodiscard]] static std::vector<UncookedShape>
  toConvexHull(const TriangleMesh &);

  ShapeBuilder &set(UncookedShape, std::optional<Transform> = std::nullopt);
  ShapeBuilder &reset();

  ShapeBuilder &add(UncookedShape, std::optional<Transform> = std::nullopt);
  ShapeBuilder &scale(const glm::vec3 &);
  ShapeBuilder &rotateTranslate(const glm::vec3 &, const glm::quat &);

  [[nodiscard]] auto size() const { return m_entries.size(); }

  [[nodiscard]] UncookedShape build() const;

private:
  [[nodiscard]] static UncookedShape _flatten(std::span<const Entry>);
  bool _flatten();

private:
  std::vector<Entry> m_entries;
};
