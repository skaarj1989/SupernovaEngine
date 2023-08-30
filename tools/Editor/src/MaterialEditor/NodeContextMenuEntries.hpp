#pragma once

#include "MaterialEditor/ShaderGraph.hpp"

using NodeFactory =
  std::function<VertexProp::Variant(ShaderGraph &, VertexDescriptor)>;
// .first = Menu item tooltip.
using NodeFactoryEx = std::pair<std::optional<std::string>, NodeFactory>;

struct MenuEntry;
// nullopt = Separator.
using MenuEntries = std::vector<std::optional<MenuEntry>>;

struct MenuEntry {
  const char *icon{nullptr};
  std::string label;

  using IsEnabled = std::function<bool(const rhi::ShaderType,
                                       const gfx::Material::Blueprint &)>;
  std::optional<IsEnabled> isEnabled{};

  using Variant = std::variant<NodeFactoryEx, MenuEntries>;
  Variant payload;
};

[[nodiscard]] MenuEntries buildNodeMenuEntries(const ScriptedFunctions &,
                                               const UserFunctions &);

std::optional<IDPair> processNodeMenu(ShaderGraph &,
                                      std::span<const std::optional<MenuEntry>>,
                                      const std::string_view pattern,
                                      const rhi::ShaderType,
                                      const gfx::Material::Blueprint &);
