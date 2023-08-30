#pragma once

#include "entt/core/type_traits.hpp"

template <class System> struct Serializer {
  template <class Archive> void save(Archive &archive) const {
    System::save(archive);
  }
  template <class Archive> void load(Archive &archive) const {
    System::load(archive);
  }
};

template <class Archive> void saveSystems(entt::type_list<>, Archive &) {}
template <class System, class... Rest, class Archive>
void saveSystems(entt::type_list<System, Rest...>, Archive &archive) {
  archive(Serializer<System>{});
  saveSystems(entt::type_list<Rest...>(), archive);
};

template <class... Systems, class Archive> void saveSystems2(Archive &archive) {
  saveSystems(entt::type_list<Systems...>(), archive);
}

template <class Archive> void loadSystems(entt::type_list<>, Archive &) {}
template <class System, class... Rest, class Archive>
void loadSystems(entt::type_list<System, Rest...>, Archive &archive) {
  archive(Serializer<System>{});
  loadSystems(entt::type_list<Rest...>(), archive);
};

template <class... Systems, class Archive> void loadSystems2(Archive &archive) {
  loadSystems(entt::type_list<Systems...>(), archive);
}
