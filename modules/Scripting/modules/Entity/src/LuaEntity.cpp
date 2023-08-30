#include "LuaEntity.hpp"
#include "sol/state.hpp"

#include "ScriptTypeInfo.hpp"
#include "MetaHelper.hpp"
#include "MetaComponent.hpp"
#include "HierarchySystem.hpp"

#include <format>
#include <vector>

void registerEntityHandle(sol::state &lua) {
  // clang-format off
  lua.new_usertype<entt::handle>("Entity",
    sol::no_constructor,

    "id", &entt::handle::entity,
    "valid", [](entt::handle self) {
      // entt::handle::vaid() does not validate the registry pointer.
      return bool(self);
    },

    "has",
      [](entt::handle self, const sol::object &type) {
        if (!self) return false; // Invalid handle.

        const auto typeId = getTypeId(type);
        if (!typeId) return false;

        const auto metaType = entt::resolve(*typeId);
        if (!metaType) return false; // Component not registered.
      
        auto result = invokeMetaFunc(metaType, MetaComponent::Functions::Has, self);
        return result.cast<bool>();
      },
    "get",
      [](entt::handle self, const sol::object &type,
         const sol::this_state s) -> sol::reference {
        if (!self) return sol::lua_nil_t{};

        const auto typeId = getTypeId(type);
        if (!typeId) return sol::lua_nil_t{};

        const auto metaType = entt::resolve(*typeId);
        if (!metaType) return sol::lua_nil_t{};

        auto result =
          invokeMetaFunc(metaType, LuaComponent::Functions::Get, self, s);
        return result.cast<sol::reference>();
      },

    "emplace",
      [](entt::handle self, sol::table comp,
         const sol::this_state s) -> sol::reference {
        if (!self) return sol::lua_nil_t{};

        const auto typeId = getTypeId(comp);
        if (!typeId) return sol::lua_nil_t{};

        const auto metaType = entt::resolve(*typeId);
        if (!metaType) return sol::lua_nil_t{};
      
        auto result = invokeMetaFunc(metaType, LuaComponent::Functions::Emplace,
                                     self, comp, s);
        return result.cast<sol::reference>();
      },
    "remove",
      [](entt::handle self, const sol::object &type) {
        if (!self) return false;

        const auto typeId = getTypeId(type);
        if (!typeId) return false;

        const auto metaType = entt::resolve(*typeId);
        auto result =
          invokeMetaFunc(metaType, MetaComponent::Functions::Remove, self);
        return result.cast<entt::handle::size_type>() > 0;
      },
    "destroy",
      [](entt::handle self) { if (self) self.destroy(); },

    "getParent",
      [](entt::handle self) {
        if (!self) return entt::handle{};
        return getParent(self).value_or(entt::handle{});
      },
    "getChildren",
      [](entt::handle self) {
        std::vector<entt::handle> childrens;
        if (!self) return childrens;

        if (auto *c = self.try_get<const ChildrenComponent>(); c) {
          childrens.reserve(c->children.size());
          std::ranges::transform(c->children, std::back_inserter(childrens),
                                 [r = self.registry()](const auto e) {
                                   return entt::handle{*r, e};
                                 });
        }
        return childrens;
      },

    "attach",
      [](entt::handle self, entt::handle newChild) {
        if (self && newChild && newChild != self)
          HierarchySystem::attachTo(*self.registry(), newChild, self);
      },
    "attachTo",
      [](entt::handle self, entt::handle newParent) {
        if (self && newParent && self != newParent)
          HierarchySystem::attachTo(*self.registry(), self, newParent);
      },
    "detach",
      [](entt::handle self) {
        if (self)
          HierarchySystem::detach(*self.registry(), self);
      },

    sol::meta_function::to_string,
      [](entt::handle h) {
        return h ? std::format("Entity({})", entt::to_integral(h.entity()))
                 : "InvalidEntity";
      }
  );
  // clang-format on
}
