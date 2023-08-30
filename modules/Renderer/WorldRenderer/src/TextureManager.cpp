#include "renderer/TextureManager.hpp"
#include "rhi/RenderDevice.hpp"
#include "LoaderHelper.hpp"

namespace gfx {

TextureManager::TextureManager(rhi::RenderDevice &rd) : m_renderDevice{rd} {}

TextureResourceHandle TextureManager::load(const std::filesystem::path &p) {
  return ::load(*this, p, LoadMode::External, m_renderDevice);
}

} // namespace gfx
