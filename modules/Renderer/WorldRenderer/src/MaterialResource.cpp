#include "renderer/MaterialResource.hpp"

namespace gfx {

MaterialResource::MaterialResource(Material &&material,
                                   const std::filesystem::path &p)
    : Resource{p}, Material{std::move(material)} {}

} // namespace gfx
