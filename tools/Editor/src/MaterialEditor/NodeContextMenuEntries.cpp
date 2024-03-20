#include "MaterialEditor/NodeContextMenuEntries.hpp"
#include "VisitorHelper.hpp"
#include "StringUtility.hpp" // contains

#include "MaterialEditor/Attribute.hpp"
#include "MaterialEditor/BuiltInConstants.hpp"
#include "MaterialEditor/FrameBlockMember.hpp"
#include "MaterialEditor/CameraBlockMember.hpp"

#include "MaterialEditor/Nodes/Append.hpp"
#include "MaterialEditor/Nodes/VectorSplitter.hpp"
#include "MaterialEditor/Nodes/MatrixSplitter.hpp"
#include "MaterialEditor/Nodes/Swizzle.hpp"
#include "MaterialEditor/Nodes/Arithmetic.hpp"
#include "MaterialEditor/Nodes/MatrixTransform.hpp"
#include "MaterialEditor/Nodes/TextureSampling.hpp"
#include "MaterialEditor/Nodes/Scripted.hpp"
#include "MaterialEditor/Nodes/Custom.hpp"

#include "MaterialEditor/NodeFactoryRegistry.hpp"
#include "MaterialEditor/FunctionAvailability.hpp"

#include "IconsFontAwesome6.h"
#include "ImGuiHelper.hpp"
#include "imgui_internal.h" // (Begin)MenuItemEx

#include "tracy/Tracy.hpp"

#include <ranges>

namespace {

// Key = Category
// Value = {{ guid, FunctionData *}}
using FunctionsByCategoryMap =
  std::map<std::string, std::vector<ScriptedFunction::Handle>, std::less<>>;

[[nodiscard]] auto
buildFunctionCategoryMap(const ScriptedFunctions &scriptedFunctions) {
  FunctionsByCategoryMap categories;
  for (const auto &[id, data] : scriptedFunctions) {
    categories[data->category].emplace_back(id, data.get());
  }
  return categories;
}

[[nodiscard]] auto shaderTypeMatch(const rhi::ShaderStages shaderStages) {
  return [shaderStages](const auto, const rhi::ShaderType shaderType) {
    return bool(shaderStages & rhi::getStage(shaderType));
  };
}

[[nodiscard]] auto hasSceneSamplers(const gfx::Material::Surface *surface,
                                    const rhi::ShaderType shaderType) {
  if (shaderType != rhi::ShaderType::Fragment) return false;
  if (surface) {
    return surface->shadingModel == gfx::ShadingModel::Lit &&
           surface->blendMode == gfx::BlendMode::Opaque &&
           surface->lightingMode == gfx::LightingMode::Transmission;
  } else {
    return true; // Postprocess domain.
  }
}

template <typename E>
  requires std::is_scoped_enum_v<E>
[[nodiscard]] auto makeFactoryInfo(E value) {
  return MenuEntry::FactoryInfo{
    hashEnum(value),
    std::format("type: {}", toString(getDataType(value))),
  };
}

} // namespace

MenuEntries buildCoreMenuEntries() {
  MenuEntries entries;
  entries.reserve(20);

  auto attributes = MenuEntries{
    MenuEntry{
      .label = "Position",
      .isEnabled = surfaceOnly,
      .payload = makeFactoryInfo(Attribute::Position),
    },
    MenuEntry{
      .label = "TexCoord0",
      .payload = makeFactoryInfo(Attribute::TexCoord0),
    },
    MenuEntry{
      .label = "TexCoord1",
      .isEnabled = surfaceOnly,
      .payload = makeFactoryInfo(Attribute::TexCoord1),
    },
    MenuEntry{
      .label = "Normal",
      .isEnabled = surfaceOnly,
      .payload = makeFactoryInfo(Attribute::Normal),
    },
    MenuEntry{
      .label = "Color",
      .isEnabled = surfaceOnly,
      .payload = makeFactoryInfo(Attribute::Color),
    },
  };
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_A,
    .label = "Attributes",
    .payload = std::move(attributes),
  });

  auto constants = MenuEntries{
    MenuEntry{.label = "bool", .payload = hashConstant<bool>()},
    MenuEntry{.label = "bvec2", .payload = hashConstant<glm::bvec2>()},
    MenuEntry{.label = "bvec3", .payload = hashConstant<glm::bvec3>()},
    MenuEntry{.label = "bvec4", .payload = hashConstant<glm::bvec4>()},

    UISeparator{},

    MenuEntry{.label = "int32", .payload = hashConstant<int32_t>()},
    MenuEntry{.label = "ivec2", .payload = hashConstant<glm::ivec2>()},
    MenuEntry{.label = "ivec3", .payload = hashConstant<glm::ivec3>()},
    MenuEntry{.label = "ivec4", .payload = hashConstant<glm::ivec4>()},

    UISeparator{},

    MenuEntry{.label = "uint32", .payload = hashConstant<uint32_t>()},
    MenuEntry{.label = "uvec2", .payload = hashConstant<glm::uvec2>()},
    MenuEntry{.label = "uvec3", .payload = hashConstant<glm::uvec3>()},
    MenuEntry{.label = "uvec4", .payload = hashConstant<glm::uvec4>()},

    UISeparator{},

    MenuEntry{.label = "float", .payload = hashConstant<float>()},
    MenuEntry{.label = "vec2", .payload = hashConstant<glm::vec2>()},
    MenuEntry{.label = "vec3", .payload = hashConstant<glm::vec3>()},
    MenuEntry{.label = "vec4", .payload = hashConstant<glm::vec4>()},

    UISeparator{},

    MenuEntry{.label = "double", .payload = hashConstant<double>()},
    MenuEntry{.label = "dvec2", .payload = hashConstant<glm::dvec2>()},
    MenuEntry{.label = "dvec3", .payload = hashConstant<glm::dvec3>()},
    MenuEntry{.label = "dvec4", .payload = hashConstant<glm::dvec4>()},

    UISeparator{},

    MenuEntry{.label = "mat2", .payload = hashConstant<glm::mat2>()},
    MenuEntry{.label = "mat3", .payload = hashConstant<glm::mat3>()},
    MenuEntry{.label = "mat4", .payload = hashConstant<glm::mat4>()},
  };
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_C,
    .label = "Constants",
    .payload = std::move(constants),
  });

  auto properties = MenuEntries{
    MenuEntry{.label = "int32", .payload = hashProperty<int32_t>()},
    MenuEntry{.label = "uint32", .payload = hashProperty<uint32_t>()},

    UISeparator{},

    MenuEntry{.label = "float", .payload = hashProperty<float>()},
    MenuEntry{.label = "vec2", .payload = hashProperty<glm::vec2>()},
    MenuEntry{.label = "vec4", .payload = hashProperty<glm::vec4>()},
  };
  entries.emplace_back(
    MenuEntry{.icon = ICON_FA_P, .label = "Properties", .payload = properties});

#define ADD_ENUM_VALUE(Enum, Value)                                            \
  MenuEntry { .label = #Value, .payload = makeFactoryInfo(Enum::Value) }

  auto frameBlock = MenuEntries{
    ADD_ENUM_VALUE(FrameBlockMember, Time),
    ADD_ENUM_VALUE(FrameBlockMember, DeltaTime),
  };
  auto cameraBlock = MenuEntries{
    ADD_ENUM_VALUE(CameraBlockMember, Projection),
    ADD_ENUM_VALUE(CameraBlockMember, InversedProjection),
    ADD_ENUM_VALUE(CameraBlockMember, View),
    ADD_ENUM_VALUE(CameraBlockMember, InversedView),
    ADD_ENUM_VALUE(CameraBlockMember, ViewProjection),
    ADD_ENUM_VALUE(CameraBlockMember, InversedViewProjection),
    ADD_ENUM_VALUE(CameraBlockMember, Resolution),
    ADD_ENUM_VALUE(CameraBlockMember, Near),
    ADD_ENUM_VALUE(CameraBlockMember, Far),

    UISeparator{},

    ADD_ENUM_VALUE(BuiltInConstant, CameraPosition),
    ADD_ENUM_VALUE(BuiltInConstant, ScreenTexelSize),
    ADD_ENUM_VALUE(BuiltInConstant, AspectRatio),
  };

  auto builtInConstants = MenuEntries{
    MenuEntry{
      .label = "ModelMatrix",
      .isEnabled = surfaceOnly,
      .payload = makeFactoryInfo(BuiltInConstant::ModelMatrix),
    },
    MenuEntry{
      .label = "FragPos (WorldSpace)",
      .isEnabled = fragmentShaderOnly,
      .payload = makeFactoryInfo(BuiltInConstant::FragPosWorldSpace),
    },
    MenuEntry{
      .label = "FragPos (ViewSpace)",
      .isEnabled = fragmentShaderOnly,
      .payload = makeFactoryInfo(BuiltInConstant::FragPosViewSpace),
    },

    UISeparator{},

    MenuEntry{
      .label = "ViewDir",
      .isEnabled = fragmentShaderOnly,
      .payload = makeFactoryInfo(BuiltInConstant::ViewDir),
    },
  };
  auto builtInSamplers = MenuEntries{
    MenuEntry{
      .label = "SceneDepth",
      .isEnabled = hasSceneSamplers,
      .payload = makeFactoryInfo(BuiltInSampler::SceneDepth),
    },
    MenuEntry{
      .label = "SceneColor",
      .isEnabled = hasSceneSamplers,
      .payload = makeFactoryInfo(BuiltInSampler::SceneColor),
    },
  };

  auto builtIns = MenuEntries{
    MenuEntry{.label = "FrameBlock", .payload = std::move(frameBlock)},
    MenuEntry{.label = "CameraBlock", .payload = std::move(cameraBlock)},

    UISeparator{},

    MenuEntry{.label = "Constants", .payload = std::move(builtInConstants)},
    MenuEntry{
      .label = "Samplers",
      .isEnabled = fragmentShaderOnly,
      .payload = std::move(builtInSamplers),
    },
  };
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_WAREHOUSE,
    .label = "Built-ins",
    .payload = std::move(builtIns),
  });

  entries.emplace_back(UISeparator{});

  auto arithmetic = MenuEntries{
    MenuEntry{
      .icon = ICON_FA_PLUS,
      .label = "Add",
      .payload = hashEnum(ArithmeticNode::Operation::Add),
    },
    MenuEntry{
      .icon = ICON_FA_MINUS,
      .label = "Subtract",
      .payload = hashEnum(ArithmeticNode::Operation::Subtract),
    },
    MenuEntry{
      .icon = ICON_FA_XMARK,
      .label = "Multiply",
      .payload = hashEnum(ArithmeticNode::Operation::Multiply),
    },
    MenuEntry{
      .icon = ICON_FA_DIVIDE,
      .label = "Divide",
      .payload = hashEnum(ArithmeticNode::Operation::Divide),
    },
  };
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_CALCULATOR,
    .label = "Arithmetic",
    .payload = std::move(arithmetic),
  });

  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_LINK,
    .label = "Append",
    .payload = typeid(AppendNode).hash_code(),
  });
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_LINK_SLASH,
    .label = "Split[vecN]",
    .payload = typeid(VectorSplitterNode).hash_code(),
  });
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_LINK_SLASH,
    .label = "Split[matNxN]",
    .payload = typeid(MatrixSplitterNode).hash_code(),
  });
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_CODE_BRANCH,
    .label = "Swizzle",
    .payload = typeid(SwizzleNode).hash_code(),
  });
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_ROTATE,
    .label = "MatrixTransform",
    .payload = typeid(MatrixTransformNode).hash_code(),
  });

  entries.emplace_back(UISeparator{});

  entries.push_back(MenuEntry{
    .icon = ICON_FA_SCROLL,
    .label = "Scripted",
    .payload = MenuEntries{},
  });
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_USER,
    .label = "User",
    .payload = MenuEntries{},
  });

  entries.emplace_back(UISeparator{});

  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_IMAGE,
    .label = "Texture",
    .payload = typeid(TextureParam).hash_code(),
  });
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_EYE_DROPPER,
    .label = "TextureSampling",
    .payload = typeid(TextureSamplingNode).hash_code(),
  });

  return entries;
}

MenuEntries buildNodeMenuEntries(const ScriptedFunctions &scriptedFunctions) {
  const auto categories = buildFunctionCategoryMap(scriptedFunctions);
  MenuEntries entries;
  entries.reserve(categories.size());
  for (const auto &[category, functions] : categories) {
    MenuEntries localEntries;
    localEntries.reserve(functions.size());
    for (const auto &[hash, data] : functions) {
      localEntries.emplace_back(MenuEntry{
        .label = data->name,
        .isEnabled = data->isEnabledCb,
        .payload = MenuEntry::FactoryInfo{hash, data->signature},
      });
    }
    entries.emplace_back(MenuEntry{
      .label = category,
      .payload = std::move(localEntries),
    });
  }
  return entries;
}
MenuEntries buildNodeMenuEntries(const UserFunctions &userFunctions) {
  MenuEntries entries;
  entries.reserve(userFunctions.size());
  for (const auto &[id, data] : userFunctions) {
    entries.emplace_back(MenuEntry{
      .label = data->name,
      .isEnabled = shaderTypeMatch(data->shaderStages),
      .payload = MenuEntry::FactoryInfo{id, buildDeclaration(*data)},
    });
  }
  return entries;
}

std::optional<MenuEntry::FactoryHash>
processNodeMenu(std::span<const MenuEntryVariant> entries,
                const gfx::Material::Surface *surface,
                const rhi::ShaderType stage, const std::string_view pattern) {
  if (entries.empty()) return std::nullopt;

  ZoneScopedN("ProcessNodeMenu");

  std::optional<MenuEntry::FactoryHash> result;

  const auto matches = [pattern](const MenuEntryVariant &variant) {
    return std::visit(
      Overload{
        // Exclude separators when filtering (avoids ugly, empty menu).
        [pattern](const UISeparator) { return pattern.empty(); },
        [pattern](const MenuEntry &entry) {
          return std::visit(
            Overload{
              [pattern, &entry](const MenuEntry::FactoryInfo &) {
                return pattern.empty() || contains(entry.label, pattern);
              },
              [](const MenuEntries &entries_) { return !entries_.empty(); },
            },
            entry.payload);
        },
      },
      variant);
  };

  auto filteredEntries = entries | std::views::filter(matches);
  if (filteredEntries.empty()) {
    ImGui::MenuItem("(empty)", nullptr, nullptr, false);
  } else {
    for (const auto &menuEntry : filteredEntries) {
      std::visit(
        Overload{
          [pattern](const UISeparator) {
            assert(pattern.empty());
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
          },
          [&](const MenuEntry &entry) {
            const auto enabled =
              entry.isEnabled ? (*entry.isEnabled)(surface, stage) : true;

            std::visit(
              Overload{
                [&](const MenuEntry::FactoryInfo &info) {
                  const auto &[factoryId, tooltip] = info;
                  if (ImGui::MenuItemEx(entry.label.data(), entry.icon, nullptr,
                                        false, enabled)) {
                    result = factoryId;
                  }
                  if (!tooltip.empty() && ImGui::IsKeyDown(ImGuiMod_Alt) &&
                      ImGui::IsItemHovered()) {
                    ImGui::ShowTooltip(tooltip.c_str());
                  }
                },
                [&](const MenuEntries &entries_) {
                  if (ImGui::BeginMenuEx(entry.label.data(), entry.icon,
                                         enabled)) {
                    result = processNodeMenu(entries_, surface, stage, pattern);
                    ImGui::EndMenu();
                  }
                },
              },
              entry.payload);
          },
        },
        menuEntry);
    }
  }
  return result;
}
