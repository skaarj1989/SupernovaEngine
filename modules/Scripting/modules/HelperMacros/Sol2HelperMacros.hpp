#pragma once

#include "entt/core/type_info.hpp" // type_hash

#define DEFINE_ENUM(Type, ...) lua.new_enum<Type>(#Type, __VA_ARGS__)

#define _MAKE_PAIR(Enum, Value)                                                \
  { #Value, Enum::Value }

#define DEFINE_USERTYPE(Type, ...) lua.new_usertype<Type>(#Type, __VA_ARGS__)

#define _BIND(Class, Member) #Member, &Class::Member

#define BIND_TYPEID(Type) "type_id", &entt::type_hash<Type>::value
#define BIND_TOSTRING(Type) sol::meta_function::to_string, [] { return #Type; }
