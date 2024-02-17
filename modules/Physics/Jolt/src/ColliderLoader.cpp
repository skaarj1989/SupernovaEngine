#include "physics/ColliderLoader.hpp"
#include "physics/ShapeSerialization.hpp"

#include "os/FileSystem.hpp"
#include "spdlog/spdlog.h"

ColliderLoader::result_type
ColliderLoader::operator()(const std::filesystem::path &p) const {
  if (const auto result = loadShape(p); result) {
    return std::make_shared<ColliderResource>(result->Get(),
                                              p.lexically_normal());
  } else {
    SPDLOG_WARN("{}: {}", os::FileSystem::relativeToRoot(p)->generic_string(),
                result.error());
    return nullptr;
  }
}
