#include "MaterialEditor/VertexProp.hpp"
#include "VisitorHelper.hpp"
#include <format>

std::string VertexProp::toString() const {
  return label.empty()
           ? std::format("[{}] {}", id, ::toString(variant))
           : std::format("[{}: {}] {}", id, label, ::toString(variant));
}

//
// Helper:
//

std::string toString(const VertexProp::Variant &variant) {
  return std::visit(
    Overload{
      [](std::monostate) -> std::string { return "(empty)"; },

      [](const MasterNodeVariant &v) -> std::string { return ::toString(v); },

      [](const ContainerNode &) -> std::string { return "Block"; },

      [](const ValueVariant &v) {
        return std::format("Value: {}", ::toString(v));
      },
      [](const Attribute a) {
        return std::format("Attribute: {}", ::toString(a));
      },
      [](const PropertyValue &v) {
        return std::format("Property: {}", gfx::toString(v));
      },
      [](const FrameBlockMember e) {
        return std::format("FrameBlock::{}", ::toString(e));
      },
      [](const CameraBlockMember e) {
        return std::format("CameraBlock::{}", ::toString(e));
      },
      [](const BuiltInConstant e) {
        return std::format("Built-in: {}", ::toString(e));
      },
      [](const BuiltInSampler e) {
        return std::format("Built-in: {}", ::toString(e));
      },
      [](const TextureParam &) -> std::string { return "Texture"; },
      [](const SplitVector e) -> std::string { return getPostfix(e); },
      [](const SplitMatrix e) {
        return std::format("Column[{}]", std::to_underlying(e));
      },
      [](const CompoundNodeVariant &v) -> std::string { return ::toString(v); },
    },
    variant);
}
std::optional<const char *> getUserLabel(const VertexProp &v) {
  return !v.label.empty() ? std::optional{v.label.c_str()} : std::nullopt;
}

void remove(ShaderGraph &g, CompoundNodeVariant &variant) {
  std::visit([&g](auto &arg) { arg.remove(g); }, variant);
}
