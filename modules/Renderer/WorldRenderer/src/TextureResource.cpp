#include "renderer/TextureResource.hpp"

namespace gfx {

TextureResource::TextureResource(Texture &&texture,
                                 const std::filesystem::path &p)
    : Resource{p}, Texture{std::move(texture)} {}

} // namespace gfx
