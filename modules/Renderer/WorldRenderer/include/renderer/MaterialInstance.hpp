#pragma once

#include "MaterialManager.hpp"

namespace gfx {

class MaterialInstance final {
public:
  explicit MaterialInstance(std::shared_ptr<Material> = {});
  MaterialInstance(const MaterialInstance &);
  MaterialInstance(MaterialInstance &&) noexcept = default;
  ~MaterialInstance() = default;

  MaterialInstance &operator=(const MaterialInstance &) = delete;
  MaterialInstance &operator=(MaterialInstance &&) noexcept = default;

  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] const Material *operator->() const noexcept;

  [[nodiscard]] const std::shared_ptr<Material> &getPrototype() const;

  // ---

  [[nodiscard]] bool hasProperties() const;

  MaterialInstance &setProperty(const std::string_view,
                                const Property::Value &);
  MaterialInstance &setTexture(const std::string_view alias,
                               std::shared_ptr<rhi::Texture>);

  MaterialInstance &setFlags(const MaterialFlags);
  MaterialInstance &setCastShadow(const bool);
  MaterialInstance &setReceiveShadow(const bool);

  MaterialInstance &enable(const bool);

  // ---

  [[nodiscard]] Property::Value getProperty(const std::string_view) const;
  [[nodiscard]] std::vector<Property> &getProperties();
  [[nodiscard]] const std::vector<Property> &getProperties() const;

  [[nodiscard]] bool hasTextures() const;
  [[nodiscard]] const TextureResources &getTextures() const;

  [[nodiscard]] MaterialFlags getFlags() const;
  [[nodiscard]] bool castsShadow() const;
  [[nodiscard]] bool receivesShadow() const;

  [[nodiscard]] bool isEnabled() const;

  // ---

  MaterialInstance &reset();

  // -- Serialization:

  template <class Archive> void save(Archive &archive) const {
    archive(serialize(m_prototype));
    if (m_prototype) {
      archive(m_properties, m_textures);
    }
    archive(m_flags);
  }
  template <class Archive> void load(Archive &archive) {
    std::optional<std::string> path;
    archive(path);
    if (path) {
      m_prototype = loadResource<MaterialManager>(*path);
      _initialize();
      archive(m_properties, m_textures);
    }
    archive(m_flags);
  }

private:
  void _initialize();

private:
  std::shared_ptr<Material> m_prototype;
  std::vector<Property> m_properties;
  TextureResources m_textures;
  MaterialFlags m_flags{MaterialFlags::None};
};

} // namespace gfx
