#pragma once

#include "NodeBase.hpp"
#include <format>

template <typename T>
concept has_toString = requires(T t) {
  { ::toString(t) } -> std::convertible_to<std::string>;
};

template <typename T> class EmbeddedNode : public NodeBase {
public:
  EmbeddedNode() = default;
  EmbeddedNode(ShaderGraph &g, const IDPair vertex, T value_)
      : NodeBase{g, vertex, Flags::Output}, value{std::move(value_)} {}

  std::unique_ptr<NodeBase> clone(const IDPair cloned) const override {
    return std::make_unique<EmbeddedNode<T>>(*graph, cloned, value);
  }

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override {
    if constexpr (has_toString<T>) {
      return std::format("Embedded<{}>", ::toString(value));
    } else {
      return std::format("Embedded<{}>", typeid(T).name());
    }
  }

  // ---

  // @return The previous value, or std::nullopt if not changed.
  std::optional<T> setValue(T value_) {
    if (value != value_) {
      std::swap(value, value_);
      markDirty();
      return value_;
    }
    return std::nullopt;
  }

  // ---

  T value;

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<NodeBase>(this));
    if constexpr (!std::is_empty_v<T>) archive(value);
  }
};
