#include "renderer/MaterialInstance.hpp"
#include "spdlog/spdlog.h"

namespace gfx {

namespace {

[[nodiscard]] auto findProperty(const std::string_view name,
                                const std::optional<std::size_t> alternative,
                                auto &properties) {
  return std::find_if(
    properties.begin(), properties.end(), [name, &alternative](const auto &p) {
      return (!alternative || *alternative == p.value.index()) &&
             name == p.name;
    });
}

} // namespace

//
// MaterialInstance class:
//

MaterialInstance::MaterialInstance(std::shared_ptr<Material> material)
    : m_prototype{material} {
  _initialize();
}
MaterialInstance::MaterialInstance(const MaterialInstance &other) = default;

MaterialInstance::operator bool() const { return m_prototype != nullptr; }

const Material *MaterialInstance::operator->() const noexcept {
  return m_prototype.get();
}

const std::shared_ptr<Material> &MaterialInstance::getPrototype() const {
  return m_prototype;
}

bool MaterialInstance::hasProperties() const { return !m_properties.empty(); }

MaterialInstance &MaterialInstance::setProperty(const std::string_view name,
                                                const Property::Value &v) {
  if (auto it = findProperty(name, v.index(), m_properties);
      it != m_properties.cend()) {
    it->value = v;
  } else {
    SPDLOG_WARN("Property '{} {}' not found", toString(v), name);
  }
  return *this;
}
MaterialInstance &
MaterialInstance::setTexture(const std::string_view alias,
                             std::shared_ptr<rhi::Texture> texture) {
  if (!texture || !*texture) {
    SPDLOG_WARN("Invalid texture");
    return *this;
  }
  if (auto it = m_textures.find(alias); it != m_textures.cend()) {
    auto &[type, ptr] = it->second;
    if (type == rhi::TextureType::Undefined || texture->getType() == type) {
      ptr = texture;
    } else {
      SPDLOG_WARN("Texture type mismatch");
    }
  } else {
    SPDLOG_WARN("Texture not found");
  }
  return *this;
}

MaterialInstance &MaterialInstance::setFlags(const MaterialFlags f) {
  m_flags = f;
  return *this;
}

#define SET_FLAG(EnumValue, Enabled)                                           \
  enabled ? (m_flags |= MaterialFlags::EnumValue)                              \
          : (m_flags &= ~MaterialFlags::EnumValue)

MaterialInstance &MaterialInstance::setCastShadow(const bool enabled) {
  SET_FLAG(CastShadow, enabled);
  return *this;
}
MaterialInstance &MaterialInstance::setReceiveShadow(const bool enabled) {
  SET_FLAG(ReceiveShadow, enabled);
  return *this;
}

MaterialInstance &MaterialInstance::enable(const bool enabled) {
  SET_FLAG(Enabled, enabled);
  return *this;
}

#undef SET_FLAG

Property::Value
MaterialInstance::getProperty(const std::string_view name) const {
  if (auto it = findProperty(name, std::nullopt, m_properties);
      it != m_properties.cend()) {
    return it->value;
  }
  return {};
}
std::vector<Property> &MaterialInstance::getProperties() {
  return m_properties;
}
const std::vector<Property> &MaterialInstance::getProperties() const {
  return m_properties;
}

bool MaterialInstance::hasTextures() const { return !m_textures.empty(); }
const TextureResources &MaterialInstance::getTextures() const {
  return m_textures;
}

MaterialFlags MaterialInstance::getFlags() const { return m_flags; }
bool MaterialInstance::castsShadow() const {
  return bool(m_flags & MaterialFlags::CastShadow);
}
bool MaterialInstance::receivesShadow() const {
  return bool(m_flags & MaterialFlags::ReceiveShadow);
}

bool MaterialInstance::isEnabled() const {
  return bool(m_flags & MaterialFlags::Enabled);
}

MaterialInstance &MaterialInstance::reset() {
  _initialize();
  return *this;
}

//
// (private):
//

void MaterialInstance::_initialize() {
  if (m_prototype) {
    const auto &blueprint = m_prototype->getBlueprint();
    m_properties = blueprint.properties;
    m_textures = blueprint.defaultTextures;
    m_flags = blueprint.flags;
  }
}

} // namespace gfx
