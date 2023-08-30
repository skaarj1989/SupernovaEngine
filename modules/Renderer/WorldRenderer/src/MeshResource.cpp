#include "renderer/MeshResource.hpp"

namespace gfx {

MeshResource::MeshResource(Mesh &&mesh, const std::filesystem::path &p)
    : Resource{p}, Mesh{std::move(mesh)} {}

} // namespace gfx
