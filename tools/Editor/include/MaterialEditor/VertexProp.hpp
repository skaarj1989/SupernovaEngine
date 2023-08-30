#pragma once

#include "Nodes/Container.hpp"
#include "Property.hpp"
#include "SplitVector.hpp"
#include "SplitMatrix.hpp"
#include "CompoundNodeVariant.hpp"
#include "MasterVariant.hpp"

// Input/Output means UI nodes flow (the direction is opposed to a graph flow).
// Graph flow = from master node.
// UI flow = to master node.
enum class NodeFlags {
  None = 0,

  Input = 1 << 0,
  Output = 1 << 1,

  Internal = 1 << 2,
};
template <> struct has_flags<NodeFlags> : std::true_type {};

using PropertyValue = gfx::Property::Value;

struct VertexProp {
  int32_t id{kInvalidId};
  NodeFlags flags{NodeFlags::None};
  std::string label;

  using Variant = std::variant<
    // A special case when a scripted function doesn't have a default argument.
    // @see ScriptedNode.cpp
    std::monostate,

    MasterNodeVariant,

    // clang-format off
    ContainerNode,

    ValueVariant,
    Attribute,
    PropertyValue,
    
    FrameBlockMember,
    CameraBlockMember,
    
    BuiltInConstant,
    BuiltInSampler,
    
    SplitVector,
    SplitMatrix,

    TextureParam,

    CompoundNodeVariant
    // clang-format on
    >;
  Variant variant;

  [[nodiscard]] std::string toString() const;

  template <class Archive> void serialize(Archive &archive) {
    archive(id, flags, label);
    // The variant is serialized after vertices (see save/load in ShaderGraph).
    // (vd<->id map is required)
  }
};

[[nodiscard]] std::string toString(const VertexProp::Variant &);
[[nodiscard]] std::optional<const char *> getUserLabel(const VertexProp &);

void remove(ShaderGraph &, CompoundNodeVariant &);
