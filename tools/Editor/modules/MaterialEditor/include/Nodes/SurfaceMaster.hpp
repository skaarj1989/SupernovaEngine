#pragma once

#include "Compound.hpp"
#include "TransientVariant.hpp"
#include "renderer/Material.hpp"

class SurfaceMasterNode : public CompoundNode {
public:
  SurfaceMasterNode() = default;
  SurfaceMasterNode(ShaderGraph &, const IDPair);

  std::unique_ptr<NodeBase> clone(const IDPair) const override;

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override;

  // ---

  struct FieldInfo {
    const char *name;
    TransientVariant defaultValue;

    bool (*isAvailable)(const gfx::Material::Surface &);
  };
  static const std::vector<FieldInfo> kFields;

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<CompoundNode>(this));
  }
};
