#pragma once

#include "entt/core/type_info.hpp" // type_hash

#define _USERTYPE_AS_TABLE(Type) [#Type].get<sol::table>()

#define DEFINE_ENUM(Type, ...) new_enum<Type>(#Type, __VA_ARGS__)
#define DEFINE_NESTED_ENUM(ParentType, Type, ...)                              \
  _USERTYPE_AS_TABLE(ParentType).new_enum<ParentType::Type>(#Type, __VA_ARGS__)

#define _MAKE_PAIR(Enum, Value)                                                \
  { #Value, Enum::Value }

#define DEFINE_USERTYPE(Type, ...) new_usertype<Type>(#Type, __VA_ARGS__)

#define DEFINE_NESTED_USERTYPE(ParentType, Type, ...)                          \
  _USERTYPE_AS_TABLE(ParentType)                                               \
    .new_usertype<ParentType::Type>(#Type, __VA_ARGS__)

#define _BIND(Class, Member) #Member, &Class::Member

#define BIND_TYPEID(Type) "type_id", &entt::type_hash<Type>::value
#define BIND_TOSTRING(Type) sol::meta_function::to_string, [] { return #Type; }

#define CAPTURE_FIELD(name, defaultValue) .name = t.get_or(#name, defaultValue)
#define CAPTURE_FIELD_T(name, T, defaultValue)                                 \
  .name = t.get_or<const T &>(#name, T{defaultValue})
