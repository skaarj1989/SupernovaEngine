#include "HierarchySystem.hpp"
#include "Transform.hpp"

#include "entt/entity/registry.hpp"
#include "entt/entity/handle.hpp"

#include "spdlog/spdlog.h"

namespace {

void initChildren(entt::registry &r, const entt::entity e) {
  for (auto child : r.get<const ChildrenComponent>(e).children) {
    HierarchySystem::attachTo(r, child, e);
  }
}
void destroyChildren(entt::registry &r, const entt::entity e) {
  // Intended copy! do not use a reference!
  const auto childrens = r.get<const ChildrenComponent>(e).children;
  for (auto child : childrens) {
    if (r.valid(child)) {
      HierarchySystem::detach(r, child);
      r.destroy(child);
    }
  }
}

void setParentTransform(entt::registry &r, const entt::entity e) {
  if (const auto *p = r.try_get<const ParentComponent>(e); p)
    if (auto *xf = r.try_get<Transform>(e); xf)
      xf->setParent(r.try_get<const Transform>(p->parent));
}
void detachTransform(entt::registry &r, const entt::entity e) {
  if (auto *xf = r.try_get<Transform>(e); xf) xf->setParent(nullptr);
}
void detachChildrenTransform(entt::registry &r, const entt::entity e) {
  if (const auto *c = r.try_get<const ChildrenComponent>(e); c)
    for (const auto child : c->children)
      detachTransform(r, child);
}

} // namespace

//
// HierarchySystem class:
//

void HierarchySystem::setup(entt::registry &r) {
  r.on_destroy<ParentComponent>().connect<&detach>();

  r.on_construct<ChildrenComponent>().connect<&initChildren>();
  r.on_destroy<ChildrenComponent>().connect<&destroyChildren>();

  r.on_construct<Transform>().connect<&setParentTransform>();
  r.on_update<Transform>().connect<&setParentTransform>();
  r.on_destroy<Transform>().connect<&detachChildrenTransform>();
}
void HierarchySystem::attachTo(entt::registry &r, const entt::entity child,
                               const entt::entity designatedParent) {
  assert(child != designatedParent);
  assert(r.valid(designatedParent) && r.valid(child));

  auto &[parent] = r.get_or_emplace<ParentComponent>(child);
  if (parent != entt::null) detach(r, child);

  parent = designatedParent;
  r.get_or_emplace<ChildrenComponent>(designatedParent).children.insert(child);

  SPDLOG_TRACE("{} attached to {}", entt::to_integral(child),
               entt::to_integral(designatedParent));

  setParentTransform(r, child);
}
void HierarchySystem::detach(entt::registry &r, const entt::entity e) {
  if (auto *pc = r.try_get<ParentComponent>(e); pc) {
    auto &[parent] = *pc;
    if (entt::null == parent) return;
    assert(r.valid(parent));

    SPDLOG_TRACE("{} detached from {}", entt::to_integral(e),
                 entt::to_integral(parent));

    // Remove self from the parent.
    if (auto *cc = r.try_get<ChildrenComponent>(parent); cc) {
      cc->children.erase(e);
    }
    parent = entt::null;
  }

  detachTransform(r, e);
}

std::optional<entt::handle> getParent(const entt::handle h) {
  const auto *c = h.try_get<const ParentComponent>();
  if (c && c->parent != entt::null) {
    return entt::handle{*h.registry(), c->parent};
  }
  return std::nullopt;
}
