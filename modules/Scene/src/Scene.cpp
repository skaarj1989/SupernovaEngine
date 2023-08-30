#include "Scene.hpp"
#include "os/FileSystem.hpp"

#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "AnimationSystem.hpp"
#include "ScriptSystem.hpp"

#include "entt/entity/snapshot.hpp"

#include "cereal/archives/json.hpp"
#include "math/Serialization.hpp"
#include "cereal/types/optional.hpp"
#include "cereal/types/variant.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/unordered_set.hpp"

#include "SystemSerializationUtility.hpp" // {save/load}Systems

void copyRegistry(entt::registry &src, entt::registry &dst) {
  dst.assign(src.data(), src.data() + src.size(), src.released());
  for (auto [id, srcStorage] : src.storage()) {
    for (auto [e] : src.storage<entt::entity>().each()) {
      auto dstStorage = dst.storage(id);
      if (auto it = dstStorage->begin(); it != dstStorage->end()) {
        dstStorage->push(e, srcStorage.value(e));
      }
    }
  }
}

namespace {

constexpr entt::type_list<PhysicsSystem, RenderSystem, AnimationSystem,
                          ScriptSystem>
  kSystemTypes{};

template <class Archive>
void serializeCoreComponents(auto &snapshot, Archive &archive) {
  // The order of components is important!
  // Transform MUST BE (de)serialized before ChildrenComponent!
  snapshot.get<entt::entity>(archive)
    .get<Transform>(archive)
    .get<NameComponent>(archive)
    .get<ChildrenComponent>(archive);
}

bool serialize(const entt::registry &r, std::ostream &os) {
  try {
    auto snapshot = entt::snapshot{r};
    OutputContext outputContext{r, snapshot};
    cereal::UserDataAdapter<OutputContext, cereal::JSONOutputArchive> archive{
      outputContext, os};

    serializeCoreComponents(snapshot, archive);
    saveSystems(kSystemTypes, archive);
  } catch (const std::exception &e) {
    return false;
  }
  return true;
}
bool deserialize(std::istream &is, entt::registry &r) {
  r.clear();

  try {
    auto snapshotLoader = entt::snapshot_loader{r};
    InputContext inputContext{r, snapshotLoader};
    cereal::UserDataAdapter<InputContext, cereal::JSONInputArchive> archive{
      inputContext, is};

    serializeCoreComponents(snapshotLoader, archive);
    loadSystems(kSystemTypes, archive);

    snapshotLoader.orphans();
  } catch (const std::exception &e) {
    r.clear();
    return false;
  }
  return true;
}

std::ostream &operator<<(std::ostream &os, const entt::registry &r) {
  serialize(r, os);
  return os;
}
std::istream &operator>>(std::istream &is, entt::registry &r) {
  deserialize(is, r);
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

Scene::Scene() = default;
Scene::Scene(const Scene &other) {
  std::stringstream ss;
  ss << other.m_registry;
  ss >> m_registry;
}
Scene::~Scene() { clear(); }

Scene &Scene::operator=(const Scene &rhs) {
  std::stringstream ss;
  ss << rhs.m_registry;
  ss >> m_registry;
  return *this;
}

entt::registry &Scene::getRegistry() { return m_registry; }
const entt::registry &Scene::getRegistry() const { return m_registry; }

void Scene::copyFrom(const Scene &src) {
  std::stringstream ss;
  ss << src.m_registry;
  ss >> m_registry;
}

entt::handle Scene::createEntity(std::optional<std::string> name) {
  auto h = get(m_registry.create());
  if (name) h.emplace<NameComponent>(std::move(*name));
  return h;
}
entt::handle Scene::get(entt::entity e) {
  return m_registry.storage<entt::entity>().contains(e)
           ? entt::handle{m_registry, e}
           : entt::handle{};
}
entt::handle Scene::clone(entt::handle in) {
  return get(::clone(m_registry, in));
}

PhysicsWorld *Scene::getPhysicsWorld() {
  auto &ctx = m_registry.ctx();
  return ctx.contains<PhysicsWorld>() ? ctx.find<PhysicsWorld>() : nullptr;
}

void Scene::clear() { m_registry.clear(); }

bool Scene::save(const std::filesystem::path &p) const {
  if (std::ostringstream ss; serialize(m_registry, ss)) {
    return os::FileSystem::saveText(p, ss.str());
  }
  return false;
}
bool Scene::load(const std::filesystem::path &p) {
  if (auto text = os::FileSystem::readText(p); text) {
    std::istringstream ss{*text};
    return deserialize(ss, m_registry);
  }
  return false;
}
