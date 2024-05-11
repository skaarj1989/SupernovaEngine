#pragma once

#include "ScriptedFunction.hpp"
#include "UserFunction.hpp"
#include <optional>

struct MenuEntry;

using UISeparator = std::monostate;
using MenuEntryVariant = std::variant<UISeparator, MenuEntry>;
using MenuEntries = std::vector<MenuEntryVariant>;

struct MenuEntry {
  const char *icon{nullptr};
  std::string label;

  using IsEnabled =
    std::function<bool(const gfx::Material::Surface *, const rhi::ShaderType)>;
  std::optional<IsEnabled> isEnabled{};

  using FactoryHash = std::size_t;
  struct FactoryInfo {
    FactoryInfo(const FactoryHash id, std::string tooltip_ = "")
        : factoryId{id}, tooltip{std::move(tooltip_)} {}

    FactoryHash factoryId;
    std::string tooltip;
  };
  using Variant = std::variant<FactoryInfo, MenuEntries>;
  Variant payload;
};

[[nodiscard]] MenuEntries buildCoreMenuEntries();
[[nodiscard]] MenuEntries buildNodeMenuEntries(const ScriptedFunctions &);
[[nodiscard]] MenuEntries buildNodeMenuEntries(const UserFunctions &);

std::optional<MenuEntry::FactoryHash>
processNodeMenu(std::span<const MenuEntryVariant>,
                const gfx::Material::Surface *, const rhi::ShaderType,
                const std::string_view pattern);
