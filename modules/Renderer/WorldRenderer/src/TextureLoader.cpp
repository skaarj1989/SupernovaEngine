#include "renderer/TextureLoader.hpp"
#include "os/FileSystem.hpp"
#include "STBImageLoader.hpp"
#include "KTXLoader.hpp"

#include "entt/core/hashed_string.hpp"
#include "spdlog/spdlog.h"

namespace gfx {

namespace {

[[nodiscard]] std::expected<rhi::Texture, std::string>
tryLoad(const std::filesystem::path &p, rhi::RenderDevice &rd) {
  const auto ext = os::FileSystem::getExtension(p);
  if (!ext) {
    return std::unexpected{"No extension."};
  }

  switch (entt::hashed_string{ext->c_str()}) {
    using namespace entt::literals;

  case ".jpg"_hs:
  case ".jpeg"_hs:
  case ".png"_hs:
  case ".tga"_hs:
  case ".bmp"_hs:
  case ".psd"_hs:
  case ".gif"_hs:
  case ".hdr"_hs:
  case ".pic"_hs:
    return loadTextureSTB(p, rd);

  case ".ktx"_hs:
  case ".ktx2"_hs:
    return loadTextureKTX(p, rd);
  }
  return std::unexpected{std::format("Unsupported extension: '{}'", *ext)};
}

} // namespace

TextureLoader::result_type
TextureLoader::operator()(const std::filesystem::path &p,
                          rhi::RenderDevice &rd) const {
  if (auto texture = tryLoad(p, rd); texture) {
    return rhi::makeShared<TextureResource>(rd, std::move(texture.value()), p);
  } else {
    SPDLOG_ERROR("Texture loading failed. {}", texture.error());
    return {};
  }
}
TextureLoader::result_type
TextureLoader::operator()(rhi::Texture &&texture) const {
  return std::make_shared<TextureResource>(std::move(texture), "");
}

} // namespace gfx
