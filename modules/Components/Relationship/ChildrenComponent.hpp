#pragma once

#include "entt/entity/entity.hpp"
#include <unordered_set>

struct ChildrenComponent {
  std::unordered_set<entt::entity> children;

  template <class Archive> void serialize(Archive &archive) {
    archive(children);
  }
};
