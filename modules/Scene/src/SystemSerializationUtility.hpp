#pragma once

#include "entt/core/type_traits.hpp"

template <class T, class... Rest>
static void collectTypes(const entt::registry &r,
                         std::vector<entt::id_type> &out,
                         entt::type_list<T, Rest...>) {
  if (const auto *storage = r.storage<T>(); storage && !storage->empty()) {
    out.push_back(storage->type().hash());
  }
  if constexpr (const auto typeList = entt::type_list<Rest...>{};
                typeList.size > 0) {
    collectTypes(r, out, typeList);
  }
}

//
// Save:
//

template <class Archive, class T, class... Rest>
static void saveComponents(Archive &archive, entt::type_list<T, Rest...>) {
  auto &[registry, snapshot] = cereal::get_user_data<OutputContext>(archive);
  if (const auto *storage = registry.storage<T>();
      storage && !storage->empty()) {
    snapshot.get<T>(archive);
  }
  if constexpr (const auto typeList = entt::type_list<Rest...>{};
                typeList.size > 0) {
    saveComponents(archive, typeList);
  }
}

template <class System, class Archive>
constexpr auto has_save = requires(Archive &archive) {
  sizeof(decltype(System::save(archive)) *) != 0;
};

template <class Archive, class System> void saveSystem(Archive &archive) {
  if constexpr (has_save<System, Archive>) {
    System::save<Archive>(archive);
  }
  auto &[registry, _] = cereal::get_user_data<OutputContext>(archive);
  std::vector<entt::id_type> types;
  types.reserve(System::kIntroducedComponents.size);
  collectTypes(registry, types, System::kIntroducedComponents);
  archive(types);

  saveComponents<Archive>(archive, System::kIntroducedComponents);
}
template <class Archive, class System, class... Rest>
void saveSystems(Archive &archive, entt::type_list<System, Rest...>) {
  saveSystem<Archive, System>(archive);
  if constexpr (const auto typeList = entt::type_list<Rest...>{};
                typeList.size > 0) {
    saveSystems<Archive>(archive, typeList);
  }
}

//
// Load:
//

template <class Archive, class T, class... Rest>
static void loadComponent(Archive &archive, const entt::id_type type,
                          entt::type_list<T, Rest...>) {
  if (entt::type_hash<T>().value() == type) {
    auto &[_, snapshotLoader] = cereal::get_user_data<InputContext>(archive);
    snapshotLoader.get<T>(archive);
  } else {
    if constexpr (const auto typeList = entt::type_list<Rest...>{};
                  typeList.size > 0) {
      loadComponent(archive, type, typeList);
    }
  }
}

template <class System, class Archive>
constexpr auto has_load = requires(Archive &archive) {
  sizeof(decltype(System::load(archive)) *) != 0;
};

template <class Archive, class System> void loadSystem(Archive &archive) {
  if constexpr (has_load<System, Archive>) {
    System::load<Archive>(archive);
  }
  std::vector<entt::id_type> types;
  types.reserve(System::kIntroducedComponents.size);
  archive(types);

  for (const auto id : types) {
    loadComponent<Archive>(archive, id, System::kIntroducedComponents);
  }
}
template <class Archive, class System, class... Rest>
void loadSystems(Archive &archive, entt::type_list<System, Rest...>) {
  loadSystem<Archive, System>(archive);
  if constexpr (const auto typeList = entt::type_list<Rest...>{};
                typeList.size > 0) {
    loadSystems<Archive>(archive, typeList);
  }
}
