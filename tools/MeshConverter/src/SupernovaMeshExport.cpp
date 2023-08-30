#include "SupernovaMeshExport.hpp"
#include "MeshExporter.hpp"
#include "assimp/Exceptional.h"

void supernovaMeshExportFunction(const char *filePath, Assimp::IOSystem *,
                                 const aiScene *scene,
                                 const Assimp::ExportProperties *properties) {
  const auto p = std::filesystem::path{filePath};
  if (!p.has_filename()) {
    throw DeadlyExportError{"Invalid path."};
  }
  const auto outputDir = p.parent_path() / "out";
  std::filesystem::remove_all(outputDir);
  const auto outputMeta = (outputDir / p.stem()).replace_extension("mesh");

  using enum MeshExporter::Flags;
  auto flags = None;
  if (properties) {
    if (properties->GetPropertyBool("ignoreMaterials")) {
      flags |= IgnoreMaterials;
    }
    if (properties->GetPropertyBool("generateLODs")) {
      flags |= GenerateLODs;
    }
  }
  MeshExporter{flags}.load(scene).save(outputMeta);
}
