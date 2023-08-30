#pragma once

#include "entt/meta/resolve.hpp"

template <typename... Args>
inline auto invokeMetaFunc(entt::meta_type metaType, entt::id_type functionId,
                           Args &&...args) {
  assert(metaType);
  if (auto metaFunction = metaType.func(functionId); metaFunction) {
    return metaFunction.invoke({}, std::forward<Args>(args)...);
  }
  return entt::meta_any{};
}

template <typename... Args>
inline auto invokeMetaFunc(entt::id_type typeId, entt::id_type functionId,
                           Args &&...args) {
  return invokeMetaFunc(entt::resolve(typeId), functionId,
                        std::forward<Args>(args)...);
}
