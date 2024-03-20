#pragma once

#include "ScopedEnumFlags.hpp"
#include "rhi/ShaderType.hpp"
#include "MaterialEditor/ShaderGraphCommon.hpp"
#include "NodeVisitor.hpp"
#include "entt/signal/emitter.hpp"
#include "cereal/types/base_class.hpp"

#include <string>

class ShaderGraph;

struct NodeLabelUpdatedEvent {};
struct NodeValueUpdatedEvent {};

class NodeBase : protected entt::emitter<NodeBase> {
  friend class entt::emitter<NodeBase>;

public:
  // Graph flow: <-- Master
  //    UI flow: --> Master
  enum class Flags {
    None = 0,

    Input = 1 << 0,
    Output = 1 << 1,

    Internal = 1 << 2, // Child
  };

  NodeBase() = default;
  NodeBase(ShaderGraph &, const IDPair, const Flags);
  virtual ~NodeBase() = default;

  using entt::emitter<NodeBase>::emitter;

  using entt::emitter<NodeBase>::on;
  using entt::emitter<NodeBase>::erase;

  // @param cloned New vertex given by `ShaderGraph`.
  virtual std::unique_ptr<NodeBase> clone(const IDPair cloned) const = 0;

  // @brief Use to make internal connection between self<->children nodes.
  // Do not forget to `updateVertexDescriptor` (only the `id` is serialized).
  virtual void joinChildren() {}
  virtual void collectChildren(std::vector<VertexDescriptor> &) const {}

  virtual void accept(NodeVisitor &) = 0;
  virtual void accept(NodeVisitor &) const = 0;

  virtual std::string toString() const = 0;

  // ---

  rhi::ShaderType getOrigin() const;

  // @return The previous label, or std::nullopt if not changed.
  std::optional<std::string> setLabel(std::string);
  void markDirty();

  bool isLinkedToRoot() const;
  bool hasConnectedInput() const;

  // ---

  ShaderGraph *graph{nullptr};
  IDPair vertex{};
  Flags flags{Flags::None};
  std::string label;

  // !WARNING!
  // Do not mix `save/load` with `serialize` (in derived classes).
  // Doing so will result with the following (misleading) runtime exception:
  // "Trying to save an unregistered polymorphic type (class ...)"
  template <class Archive> void serialize(Archive &archive) {
    archive(vertex, flags, label);
  }
};

template <> struct has_flags<NodeBase::Flags> : std::true_type {};
