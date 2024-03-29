#include "physics/ShapeSerialization.hpp"
#include "os/FileSystem.hpp"

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "Jolt/Physics/Collision/PhysicsMaterial.h"
#include "Jolt/Core/StreamWrapper.h"
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

#include "tracy/Tracy.hpp"

#include <cassert>

namespace {

constexpr char kMagicId[] = "JPH::Shape";
constexpr auto kMagicIdSize = sizeof(kMagicId);

} // namespace

bool saveShape(const std::filesystem::path &p,
               const JPH::ShapeSettings::ShapeResult &uncooked) {
  assert(uncooked.IsValid());
  ZoneScopedN("SerializeShape");

  std::ostringstream oss;
  JPH::StreamOutWrapper writer(oss);
  writer.Write(kMagicId);

  JPH::Shape::ShapeToIDMap shapeToId;
  JPH::Shape::MaterialToIDMap materialToId;
  uncooked.Get()->SaveWithChildren(writer, shapeToId, materialToId);

  return os::FileSystem::saveText(p, oss.str());
}
std::expected<JPH::Shape::ShapeResult, std::string>
loadShape(const std::filesystem::path &p) {
  ZoneScopedN("DeserializeShape");
  const auto text = os::FileSystem::readText(p);
  if (!text) {
    return std::unexpected{text.error()};
  }
  std::istringstream iss{*text};
  JPH::StreamInWrapper reader{iss};
  std::remove_const_t<decltype(kMagicId)> magicId{};
  reader.Read(magicId);
  if (strncmp(magicId, kMagicId, kMagicIdSize) != 0)
    return std::unexpected{"Invalid Shape signature."};

  JPH::Shape::IDToShapeMap idToShape;
  JPH::Shape::IDToMaterialMap idToMaterial;
  const auto result =
    JPH::Shape::sRestoreWithChildren(reader, idToShape, idToMaterial);
  if (result.HasError()) {
    return std::unexpected{result.GetError().c_str()};
  }
  return result;
}
