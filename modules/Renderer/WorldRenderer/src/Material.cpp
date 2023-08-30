#include "renderer/Material.hpp"
#include "StringUtility.hpp"
#include "math/Hash.hpp"
#include "glm/common.hpp" // ceil, max

namespace std {

template <> struct hash<gfx::DecalBlendMode> {
  auto operator()(const gfx::DecalBlendMode &v) const noexcept {
    size_t h{0};
    hashCombine(h, v.normal, v.emissive, v.albedo, v.metallicRoughnessAO);
    return h;
  }
};
template <> struct hash<gfx::Material::Surface> {
  auto operator()(const gfx::Material::Surface &v) const noexcept {
    size_t h{0};
    hashCombine(h, v.shadingModel, v.blendMode, v.decalBlendMode,
                v.lightingMode, v.cullMode);
    return h;
  }
};

template <> struct hash<gfx::Material::Blueprint::Code> {
  auto operator()(const gfx::Material::Blueprint::Code &v) const noexcept {
    size_t h{0};
    hashCombine(h, v.source, v.includes);
    for (const auto &str : v.defines) {
      hashCombine(h, str);
    }
    return h;
  }
};

template <> struct hash<gfx::Material::Blueprint> {
  auto operator()(const gfx::Material::Blueprint &v) const noexcept {
    size_t h{0};
    if (v.surface) hashCombine(h, *v.surface);
    for (const auto &p : v.properties) {
      hashCombine(h, p.value.index());
    }
    for (const auto &[_, t] : v.defaultTextures) {
      hashCombine(h, t.type);
    }
    hashCombine(h, v.userVertCode, v.userFragCode, v.flags);
    return h;
  }
};

} // namespace std

namespace gfx {

namespace {

[[nodiscard]] auto
buildPropertyLayout(const std::vector<Property> &properties) {
  PropertyLayout layout;
  if (properties.empty()) return layout;

  std::size_t offset{0};
  std::size_t size{0};

  std::size_t alignment{0};

  for (const auto &p : properties) {
    size = getSize(p.value);
    assert(size > 0);

    const auto leftOver = offset % size;
    offset = leftOver > 0 ? offset + (size - leftOver) : offset;

    layout.members.emplace_back(uint8_t(offset), uint8_t(size));
    offset += size;

    alignment = glm::max(alignment, size);
  }
  assert(offset > 0);

  layout.stride =
    std::size_t(glm::ceil(float(offset) / float(alignment))) * alignment;

  return layout;
}

void fix(Material::Surface &surface) {
  if (surface.lightingMode == LightingMode::Transmission) {
    surface.blendMode = BlendMode::Opaque;
  }
}

} // namespace

bool operator==(const TextureResources &a, const TextureResources &b) {
  return a.size() == b.size() && std::equal(a.cbegin(), a.cend(), b.begin());
}

//
// Material class:
//

bool operator<(const Material &a, const Material &b) {
  return a.getHash() < b.getHash();
}

std::string_view Material::getName() const { return m_name; }
std::size_t Material::getHash() const { return m_hash; }
const Material::Blueprint &Material::getBlueprint() const {
  return m_blueprint;
}
const PropertyLayout &Material::getPropertyLayout() const {
  return m_propertyLayout;
}

//
// (private):
//

Material::Material(std::string &&name, std::size_t hash,
                   const Blueprint &blueprint)
    : m_name{std::move(name)}, m_hash{hash}, m_blueprint{blueprint},
      m_propertyLayout{buildPropertyLayout(blueprint.properties)} {}

//
// Builder class:
//

using Builder = Material::Builder;

Builder &Builder::setName(const std::string_view name) {
  m_name = name;
  return *this;
}

Builder &Builder::setBlueprint(Blueprint b) {
  m_blueprint = std::move(b);
  return *this;
}

Builder &Builder::setDomain(MaterialDomain domain) {
  return domain == MaterialDomain::Surface ? setSurface(Surface{})
                                           : setPostProcess();
}
Builder &Builder::setSurface(Surface surface) {
  m_blueprint.surface = std::move(surface);
  return setFlags(MaterialFlags::SurfaceDefault);
}
Builder &Builder::setPostProcess() {
  m_blueprint.surface = std::nullopt;
  return setFlags(MaterialFlags::Enabled);
}

Builder &Builder::addProperty(const Property &p) {
  const auto sameName = [&p](auto &c) { return c.name == p.name; };

  if (auto it = std::ranges::find_if(m_blueprint.properties, sameName);
      it != m_blueprint.properties.cend()) {
    // Update value (if a property with the same name already exists).
    it->value = p.value;
  } else {
    m_blueprint.properties.emplace_back(p);
  }
  return *this;
}
Builder &Builder::addSampler(rhi::TextureType type, const std::string &alias,
                             std::shared_ptr<rhi::Texture> texture) {
  m_blueprint.defaultTextures[alias] = {type, texture};
  return *this;
}

Builder &Builder::setUserVertCode(Blueprint::Code code) {
  m_blueprint.userVertCode = std::move(code);
  return *this;
}
Builder &Builder::setUserFragCode(Blueprint::Code code) {
  m_blueprint.userFragCode = std::move(code);
  return *this;
}

Builder &Builder::setFlags(MaterialFlags flags) {
  m_blueprint.flags = flags;
  return *this;
}

Material Builder::build() {
  if (m_blueprint.surface) fix(*m_blueprint.surface);

  const auto h = std::hash<Material::Blueprint>{}(m_blueprint);
  return Material{std::move(m_name), h, m_blueprint};
}

//
// Helper:
//

MaterialDomain getDomain(const Material &material) {
  return getDomain(material.getBlueprint());
}
MaterialDomain getDomain(const Material::Blueprint &blueprint) {
  return blueprint.surface ? MaterialDomain::Surface
                           : MaterialDomain::PostProcess;
}

bool isSurface(const Material &material) {
  return isSurface(material.getBlueprint());
}
bool isSurface(const Material::Blueprint &blueprint) {
  return getDomain(blueprint) == MaterialDomain::Surface;
}

const Material::Surface &getSurface(const Material &material) {
  return *material.getBlueprint().surface;
}

bool hasProperties(const Material &material) {
  return !material.getBlueprint().properties.empty();
}

#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

const char *toString(MaterialDomain domain) {
  switch (domain) {
    using enum MaterialDomain;

    CASE(Surface);
    CASE(PostProcess);
  }
  assert(false);
  return "Undefined";
}
const char *toString(ShadingModel shadingModel) {
  switch (shadingModel) {
    using enum ShadingModel;

    CASE(Lit);
    CASE(Unlit);
  }
  assert(false);
  return "Undefined";
}
const char *toString(BlendMode blendMode) {
  switch (blendMode) {
    using enum BlendMode;

    CASE(Opaque);
    CASE(Masked);
    CASE(Transparent);
    CASE(Add);
    CASE(Modulate);
  }
  assert(false);
  return "Undefined";
}
const char *toString(LightingMode lightingMode) {
  switch (lightingMode) {
    using enum LightingMode;

    CASE(Default);
    CASE(Transmission);
  }
  assert(false);
  return "Undefined";
}

std::string toString(MaterialFlags flags) {
  const auto values = getValues(flags);
  return values.empty() ? "None" : join(values, ", ");
}
std::vector<const char *> getValues(MaterialFlags flags) {
  using enum MaterialFlags;
  if (flags == None) return {};

  std::vector<const char *> values;
  // There are more than two flags, but a surface should not be mixed with
  // postprocess.
  constexpr auto kMaxNumFlags = 2;
  values.reserve(kMaxNumFlags);

#define CHECK_FLAG(Value)                                                      \
  if (bool(flags & Value)) values.push_back(#Value)

  CHECK_FLAG(CastShadow);
  CHECK_FLAG(ReceiveShadow);
  CHECK_FLAG(Enabled);

  return values;
}

} // namespace gfx
