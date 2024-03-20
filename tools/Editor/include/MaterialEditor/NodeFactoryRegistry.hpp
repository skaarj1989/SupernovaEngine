#pragma once

#include "ShaderGraphCommon.hpp"
#include "ValueVariant.hpp"
#include "PropertyVariant.hpp"
#include "ScriptedFunction.hpp"
#include "UserFunction.hpp"
#include <unordered_map>

static_assert(std::is_same_v<UserFunction::ID, ScriptedFunction::ID>);

template <typename E>
[[nodiscard]] auto hashEnum(E value)
  requires std::is_scoped_enum_v<E>
{
  return typeid(E).hash_code() << static_cast<std::size_t>(value);
}
template <typename T> [[nodiscard]] auto hashConstant() {
  return typeid(ValueVariant).hash_code() + typeid(T).hash_code();
}
template <typename T> [[nodiscard]] auto hashProperty() {
  return typeid(PropertyVariant).hash_code() + typeid(T).hash_code();
}

class ShaderGraph;

using NodeFactory =
  std::function<IDPair(ShaderGraph &, std::optional<VertexID>)>;

class NodeFactoryRegistry {
public:
  NodeFactoryRegistry();

  using FactoryID = std::size_t;

  template <typename Data>
  using Functions = std::unordered_map<FactoryID, std::unique_ptr<Data>>;

  template <class Data> auto load(const Functions<Data> &functions) {
    std::size_t count{0};
    for (const auto &[id, data] : functions) {
      if (add({id, data.get()})) ++count;
    }
    return count;
  }

  bool add(const ScriptedFunction::Handle);
  bool add(const UserFunction::Handle);

  template <class Data> auto unload(const Functions<Data> &functions) {
    std::size_t count{0};
    for (const auto &[id, _] : functions) {
      if (remove(id)) ++count;
    }
    return count;
  }

  bool remove(const FactoryID);

  IDPair create(const FactoryID, ShaderGraph &, std::optional<VertexID>) const;

private:
  using Storage = std::unordered_map<FactoryID, NodeFactory>;
  Storage m_storage;
};
