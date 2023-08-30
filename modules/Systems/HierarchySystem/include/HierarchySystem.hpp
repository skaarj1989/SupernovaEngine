#pragma once

#include "entt/entity/registry.hpp"
#include "ParentComponent.hpp"
#include "ChildrenComponent.hpp"

/*
  Context variables:
  - (none)
  Components:
  - [setup callbacks] Transform
  - [setup callbacks] ParentComponent
  - [setup callbacks] ChildrenComponent
*/
class HierarchySystem {
public:
  static void setup(entt::registry &);

  static void attachTo(entt::registry &, entt::entity child,
                       entt::entity designatedParent);
  static void detach(entt::registry &, entt::entity);
};

std::optional<entt::handle> getParent(entt::handle);
