#include "Scene.hpp"
#include "os/FileSystem.hpp"

#include "cereal/archives/json.hpp"
#include "cereal/archives/binary.hpp"
#include "math/Serialization.hpp"
#include "cereal/types/optional.hpp"
#include "cereal/types/variant.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/unordered_set.hpp"

#include "SystemSerializationUtility.hpp" // {save/load}Systems

namespace {

void setupSystems(entt::registry &r, gfx::WorldRenderer &worldRenderer,
                  RmlUiRenderInterface &uiRenderInterface,
                  audio::Device &audioDevice, sol::state &lua) {
  HierarchySystem::setup(r);
  PhysicsSystem::setup(r);
  RenderSystem::setup(r, worldRenderer);
  AudioSystem::setup(r, audioDevice);
  UISystem::setup(r, uiRenderInterface);
  ScriptSystem::setup(r, lua);

  auto &env = r.ctx().get<ScriptContext>().defaultEnv;

  env["getPhysicsWorld"] = [&r] { return r.ctx().find<PhysicsWorld>(); };
  env["getAudioWorld"] = [&r] { return r.ctx().find<AudioWorld>(); };
  // clang-format off
  env["setMainListener"] = sol::overload(
    [&r](const entt::entity e) { getMainListener(r).e = e; },
    [](const entt::handle h) { getMainListener(*h.registry()).e = h; }
  );
  // clang-format on
};

constexpr entt::type_list<NameComponent, Transform, ChildrenComponent>
  kCoreTypes{};

constexpr entt::type_list<PhysicsSystem, RenderSystem, AnimationSystem,
                          AudioSystem, UISystem, ScriptSystem>
  kSystemTypes{};

template <class UnderlyingArchive>
void serialize(const entt::registry &r, std::ostream &os) {
  try {
    entt::snapshot snapshot{r};
    OutputContext outputContext{r, snapshot};
    cereal::UserDataAdapter<OutputContext, UnderlyingArchive> archive{
      outputContext, os};

    snapshot.get<entt::entity>(archive);

    std::vector<entt::id_type> types;
    types.reserve(kCoreTypes.size);
    collectTypes(r, types, kCoreTypes);
    archive(types);

    saveComponents<UnderlyingArchive>(archive, kCoreTypes);
    saveSystems<UnderlyingArchive>(archive, kSystemTypes);
  } catch (const std::exception &e) {
    os.setstate(std::ios_base::badbit);
  }
}
template <class UnderlyingArchive>
void deserialize(std::istream &is, entt::registry &r) {
  r.clear();
  try {
    entt::snapshot_loader snapshotLoader{r};
    InputContext inputContext{r, snapshotLoader};
    cereal::UserDataAdapter<InputContext, UnderlyingArchive> archive{
      inputContext, is};
    is.clear();

    snapshotLoader.get<entt::entity>(archive);

    std::vector<entt::id_type> types;
    types.reserve(kCoreTypes.size);
    archive(types);

    for (const auto id : types) {
      loadComponent<UnderlyingArchive>(archive, id, kCoreTypes);
    }
    loadSystems<UnderlyingArchive>(archive, kSystemTypes);

    snapshotLoader.orphans();
  } catch (const std::exception &e) {
    is.setstate(std::ios::badbit);
  }
}

[[nodiscard]] auto getxalloc() {
  static const auto i = std::ios_base::xalloc();
  return i;
}
void setArchiveType(std::ios_base &ib, const Scene::ArchiveType archiveType) {
  ib.iword(getxalloc()) = std::to_underlying(archiveType);
}
[[nodiscard]] Scene::ArchiveType getArchiveType(std::ios_base &s) {
  return static_cast<Scene::ArchiveType>(s.iword(getxalloc()));
}

std::ostream &operator<<(std::ostream &os,
                         const Scene::ArchiveType archiveType) {
  setArchiveType(os, archiveType);
  return os;
}
std::ostream &operator<<(std::ostream &os, const entt::registry &r) {
#define CASE(Type)                                                             \
  case Type:                                                                   \
    serialize<cereal::Type##OutputArchive>(r, os);                             \
    break

  switch (getArchiveType(os)) {
    using enum Scene::ArchiveType;

    CASE(Binary);
    CASE(JSON);
  }
#undef CASE

  return os;
}

std::istream &operator>>(std::istream &is,
                         const Scene::ArchiveType archiveType) {
  setArchiveType(is, archiveType);
  return is;
}
std::istream &operator>>(std::istream &is, entt::registry &r) {
#define CASE(Type)                                                             \
  case Type:                                                                   \
    deserialize<cereal::Type##InputArchive>(is, r);                            \
    break

  switch (getArchiveType(is)) {
    using enum Scene::ArchiveType;

    CASE(Binary);
    CASE(JSON);
  }
#undef CASE

  return is;
}

[[nodiscard]] auto clone(entt::registry &r, entt::entity in) {
  constexpr auto kIgnoredTypes = std::array{
    entt::type_hash<entt::entity>::value(),
    entt::type_hash<ParentComponent>::value(),
    entt::type_hash<ChildrenComponent>::value(),
  };
  constexpr auto equals = [](auto v) { return [v](auto e) { return e == v; }; };

  auto out = r.create();
  for (auto &&[componentId, pool] : r.storage()) {
    if (std::ranges::any_of(kIgnoredTypes, equals(componentId))) {
      continue;
    }
    if (pool.contains(in)) {
      const auto it = pool.push(out, pool.value(in));
      const auto emplaced = it != pool.cend();
      // A component has to be copy constructible.
      assert(emplaced);
    }
  }
  return out;
}

} // namespace

//
// Scene class:
//

Scene::Scene(gfx::WorldRenderer &worldRenderer,
             RmlUiRenderInterface &uiRenderInterface,
             audio::Device &audioDevice, sol::state &lua) {
  m_registry = std::make_unique<entt::registry>();
  setupSystems(*m_registry, worldRenderer, uiRenderInterface, audioDevice, lua);
}
Scene::Scene(const Scene &other) { copyFrom(other); }
Scene::~Scene() {
  if (m_registry) clear();
}

Scene &Scene::operator=(const Scene &rhs) {
  if (this != &rhs) copyFrom(rhs);
  return *this;
}

entt::registry &Scene::getRegistry() { return *m_registry; }
const entt::registry &Scene::getRegistry() const { return *m_registry; }

void Scene::copyFrom(const Scene &src) {
  m_registry = std::make_unique<entt::registry>();

  auto &srcCtx = src.m_registry->ctx();
  setupSystems(*m_registry, *srcCtx.get<gfx::WorldRenderer *>(),
               *srcCtx.get<RmlUiRenderInterface *>(),
               srcCtx.get<AudioWorld>().getDevice(),
               *srcCtx.get<ScriptContext>().lua);

  std::stringstream ss;
  ss << ArchiveType::Binary << *src.m_registry;
  ss >> *m_registry;

  auto &dstCtx = m_registry->ctx();
  dstCtx.get<MainCamera>().e = srcCtx.get<MainCamera>().e;
  dstCtx.get<MainListener>().e = srcCtx.get<MainListener>().e;
}

entt::handle Scene::createEntity(std::optional<std::string> name) {
  auto h = get(m_registry->create());
  if (name) h.emplace<NameComponent>(std::move(*name));
  return h;
}
entt::handle Scene::get(entt::entity e) {
  return m_registry->storage<entt::entity>().contains(e)
           ? entt::handle{*m_registry, e}
           : entt::handle{};
}
entt::handle Scene::clone(entt::handle in) {
  return get(::clone(*m_registry, in));
}

PhysicsWorld *Scene::getPhysicsWorld() {
  return m_registry->ctx().find<PhysicsWorld>();
}

bool Scene::empty() const {
  return m_registry->storage<entt::entity>().empty();
}
void Scene::clear() { m_registry->clear(); }

bool Scene::save(const std::filesystem::path &p,
                 const ArchiveType archiveType) const {
  std::ostringstream os;
  os << archiveType << *m_registry;
  return os.good() && os::FileSystem::saveText(p, os.str());
}
bool Scene::load(const std::filesystem::path &p,
                 const ArchiveType archiveType) {
  if (auto text = os::FileSystem::readText(p); text) {
    std::istringstream is{*text};
    is >> archiveType >> *m_registry;
    return is.good();
  }
  return false;
}
