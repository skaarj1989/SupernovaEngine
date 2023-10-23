#include "NodeContextMenuEntries.hpp"
#include "VisitorHelper.hpp"
#include "StringUtility.hpp"
#include "MaterialEditor/FunctionAvailability.hpp"

#include "CreateNodeHelper.hpp"
#include "MaterialEditor/Nodes/FrameBlock.hpp"
#include "MaterialEditor/Nodes/CameraBlock.hpp"

#include "IconsFontAwesome6.h"
#include "ImGuiHelper.hpp"
#include "imgui_internal.h"

#include "tracy/Tracy.hpp"

#include <ranges>

namespace {

// Key = Category
// Value = {{ guid, FunctionData *}}
using FunctionsByCategoryMap =
  std::map<std::string,
           std::vector<std::pair<uint32_t, const ScriptedFunctionData *>>,
           std::less<>>;

[[nodiscard]] auto
buildFunctionCategoryMap(const ScriptedFunctions &scriptedFunctions) {
  ZoneScoped;

  FunctionsByCategoryMap categories;
  for (const auto &[guid, data] : scriptedFunctions) {
    auto &v = categories[data->category];
    v.emplace_back(guid, data.get());
  }
  return categories;
}

// @param pair {.first = guid/hash, .second = FunctionData *}
template <typename Node>
[[nodiscard]] NodeFactory addFunction(const auto pair) {
  assert(pair.second);
  return [pair](ShaderGraph &g, VertexDescriptor parent) {
    return Node::create(g, parent, pair);
  };
};

[[nodiscard]] auto
makeMenuEntry(const std::pair<uint32_t, const ScriptedFunctionData *> &p) {
  const auto data = p.second;
  assert(data);
  return MenuEntry{
    .label = data->name,
    .isEnabled = data->isEnabledCb,
    .payload =
      NodeFactoryEx{
        !data->signature.empty() ? std::optional{data->signature}
                                 : std::nullopt,
        addFunction<ScriptedNode>(p),
      },
  };
}

void insertScriptedFunctionEntries(MenuEntries &entries,
                                   const ScriptedFunctions &scriptedFunctions) {
  ZoneScoped;

  const auto categories = buildFunctionCategoryMap(scriptedFunctions);
  entries.reserve(entries.size() + categories.size());

  std::ranges::transform(categories, std::back_inserter(entries),
                         [](const FunctionsByCategoryMap::value_type &p) {
                           const auto &[category, functions] = p;

                           MenuEntries localEntries;
                           localEntries.reserve(functions.size());

                           std::ranges::transform(
                             functions, std::back_inserter(localEntries),
                             makeMenuEntry);
                           return MenuEntry{
                             .label = category,
                             .payload = std::move(localEntries),
                           };
                         });
}

[[nodiscard]] auto
buildUserFunctionEntries(const UserFunctions &userFunctions) {
  static constexpr auto matchShaderType =
    [](const rhi::ShaderStages shaderStages) {
      return [shaderStages](const rhi::ShaderType shaderType, auto) {
        switch (shaderType) {
        case rhi::ShaderType::Vertex:
          return bool(shaderStages & rhi::ShaderStages::Vertex);
        case rhi::ShaderType::Fragment:
          return bool(shaderStages & rhi::ShaderStages::Fragment);
        }
        return false;
      };
    };

  MenuEntries entries;
  entries.reserve(userFunctions.size());
  std::ranges::transform(
    userFunctions, std::back_inserter(entries),
    [](const UserFunctions::value_type &p) {
      const auto &[hash, data] = p;
      return MenuEntry{
        .label = data->name,
        .isEnabled = matchShaderType(data->shaderStages),
        .payload =
          NodeFactoryEx{
            buildDeclaration(*data),
            addFunction<CustomNode>(std::pair{hash, data.get()}),
          },
      };
    });
  return entries;
}

[[nodiscard]] auto hasSceneSamplers(const rhi::ShaderType shaderType,
                                    const gfx::Material::Blueprint &blueprint) {
  if (shaderType != rhi::ShaderType::Fragment) return false;

  if (const auto &surface = blueprint.surface; surface) {
    return surface->shadingModel == gfx::ShadingModel::Lit &&
           surface->blendMode == gfx::BlendMode::Opaque &&
           surface->lightingMode == gfx::LightingMode::Transmission;
  } else {
    return true; // Postprocess domain.
  }
}

} // namespace

[[nodiscard]] MenuEntries
buildNodeMenuEntries(const ScriptedFunctions &scriptedFunctions,
                     const UserFunctions &userFunctions) {
  ZoneScoped;

#define SEPARATOR std::nullopt

  MenuEntries entries;
  entries.reserve(30);

  const auto forwardArg = [](auto v) {
    return NodeFactoryEx{
      toString(getDataType(v)),
      [v](auto &, auto) { return v; },
    };
  };

  static const auto attributes = MenuEntries{
    MenuEntry{
      .label = "Position",
      .isEnabled = surfaceOnly,
      .payload = forwardArg(Attribute::Position),
    },
    MenuEntry{
      .label = "TexCoord0",
      .payload = forwardArg(Attribute::TexCoord0),
    },
    MenuEntry{
      .label = "TexCoord1",
      .isEnabled = surfaceOnly,
      .payload = forwardArg(Attribute::TexCoord1),
    },
    MenuEntry{
      .label = "Normal",
      .isEnabled = surfaceOnly,
      .payload = forwardArg(Attribute::Normal),
    },
    MenuEntry{
      .label = "Color",
      .isEnabled = surfaceOnly,
      .payload = forwardArg(Attribute::Color),
    },
  };
  entries.emplace_back(
    MenuEntry{.icon = ICON_FA_A, .label = "Attributes", .payload = attributes});

  const auto addValueVariant = [](const ValueVariant &v) {
    return NodeFactoryEx{std::nullopt, [v](auto &, auto) { return v; }};
  };

  static const auto constants = MenuEntries{
    MenuEntry{.label = "bool", .payload = addValueVariant(false)},
    MenuEntry{.label = "bvec2", .payload = addValueVariant(glm::bvec2{false})},
    MenuEntry{.label = "bvec3", .payload = addValueVariant(glm::bvec3{false})},
    MenuEntry{.label = "bvec4", .payload = addValueVariant(glm::bvec4{false})},

    SEPARATOR,

    MenuEntry{.label = "int32", .payload = addValueVariant(0)},
    MenuEntry{.label = "ivec2", .payload = addValueVariant(glm::ivec2{0})},
    MenuEntry{.label = "ivec3", .payload = addValueVariant(glm::ivec3{0})},
    MenuEntry{.label = "ivec4", .payload = addValueVariant(glm::ivec4{0})},

    SEPARATOR,

    MenuEntry{.label = "uint32", .payload = addValueVariant(0u)},
    MenuEntry{.label = "uvec2", .payload = addValueVariant(glm::uvec2{0u})},
    MenuEntry{.label = "uvec3", .payload = addValueVariant(glm::uvec3{0u})},
    MenuEntry{.label = "uvec4", .payload = addValueVariant(glm::uvec4{0u})},

    SEPARATOR,

    MenuEntry{.label = "float", .payload = addValueVariant(0.0f)},
    MenuEntry{.label = "vec2", .payload = addValueVariant(glm::vec2{0.0f})},
    MenuEntry{.label = "vec3", .payload = addValueVariant(glm::vec3{0.0f})},
    MenuEntry{.label = "vec4", .payload = addValueVariant(glm::vec4{0.0f})},

    SEPARATOR,

    MenuEntry{.label = "double", .payload = addValueVariant(0.0)},
    MenuEntry{.label = "dvec2", .payload = addValueVariant(glm::dvec2{0.0})},
    MenuEntry{.label = "dvec3", .payload = addValueVariant(glm::dvec3{0.0})},
    MenuEntry{.label = "dvec4", .payload = addValueVariant(glm::dvec4{0.0})},

    SEPARATOR,

    MenuEntry{.label = "mat2", .payload = addValueVariant(glm::mat2{1.0f})},
    MenuEntry{.label = "mat3", .payload = addValueVariant(glm::mat3{1.0f})},
    MenuEntry{.label = "mat4", .payload = addValueVariant(glm::mat4{1.0f})},
  };
  entries.emplace_back(
    MenuEntry{.icon = ICON_FA_C, .label = "Constants", .payload = constants});

  const auto addPropertyVariant = [](const PropertyValue &v) {
    return NodeFactoryEx{std::nullopt, [v](auto &, auto) { return v; }};
  };

  static const auto properties = MenuEntries{
    MenuEntry{.label = "int32", .payload = addPropertyVariant(0)},
    MenuEntry{.label = "uint32", .payload = addPropertyVariant(0u)},

    SEPARATOR,

    MenuEntry{.label = "float", .payload = addPropertyVariant(0.0f)},
    MenuEntry{.label = "vec2", .payload = addPropertyVariant(glm::vec2{0.0f})},
    MenuEntry{.label = "vec4", .payload = addPropertyVariant(glm::vec4{0.0f})},
  };
  entries.emplace_back(
    MenuEntry{.icon = ICON_FA_P, .label = "Properties", .payload = properties});

#define ADD_ENUM_VALUE(Enum, Value)                                            \
  MenuEntry { .label = #Value, .payload = forwardArg(Enum::Value) }

  static const auto frameBlock = MenuEntries{
    MenuEntry{
      .label = "struct {}",
      .payload = NodeFactoryEx{std::nullopt, createFrameBlock},
    },

    SEPARATOR,

    ADD_ENUM_VALUE(FrameBlockMember, Time),
    ADD_ENUM_VALUE(FrameBlockMember, DeltaTime),
  };
  static const auto cameraBlock = MenuEntries{
    MenuEntry{
      .label = "struct {}",
      .payload = NodeFactoryEx{std::nullopt, createCameraBlock},
    },

    SEPARATOR,

    ADD_ENUM_VALUE(CameraBlockMember, Projection),
    ADD_ENUM_VALUE(CameraBlockMember, InversedProjection),
    ADD_ENUM_VALUE(CameraBlockMember, View),
    ADD_ENUM_VALUE(CameraBlockMember, InversedView),
    ADD_ENUM_VALUE(CameraBlockMember, ViewProjection),
    ADD_ENUM_VALUE(CameraBlockMember, InversedViewProjection),
    ADD_ENUM_VALUE(CameraBlockMember, Resolution),
    ADD_ENUM_VALUE(CameraBlockMember, Near),
    ADD_ENUM_VALUE(CameraBlockMember, Far),

    SEPARATOR,

    ADD_ENUM_VALUE(BuiltInConstant, CameraPosition),
    ADD_ENUM_VALUE(BuiltInConstant, ScreenTexelSize),
    ADD_ENUM_VALUE(BuiltInConstant, AspectRatio),
  };

  static const auto builtInConstants = MenuEntries{
    MenuEntry{
      .label = "ModelMatrix",
      .isEnabled = surfaceOnly,
      .payload = forwardArg(BuiltInConstant::ModelMatrix),
    },
    MenuEntry{
      .label = "FragPos (WorldSpace)",
      .isEnabled = fragmentShaderOnly,
      .payload = forwardArg(BuiltInConstant::FragPosWorldSpace),
    },
    MenuEntry{
      .label = "FragPos (ViewSpace)",
      .isEnabled = fragmentShaderOnly,
      .payload = forwardArg(BuiltInConstant::FragPosViewSpace),
    },

    SEPARATOR,

    MenuEntry{
      .label = "ViewDir",
      .isEnabled = fragmentShaderOnly,
      .payload = forwardArg(BuiltInConstant::ViewDir),
    },
  };
  static const auto builtInSamplers = MenuEntries{
    MenuEntry{
      .label = "SceneDepth",
      .isEnabled = hasSceneSamplers,
      .payload = forwardArg(BuiltInSampler::SceneDepth),
    },
    MenuEntry{
      .label = "SceneColor",
      .isEnabled = hasSceneSamplers,
      .payload = forwardArg(BuiltInSampler::SceneColor),
    },
  };

  static const auto builtIns = MenuEntries{
    MenuEntry{.label = "FrameBlock", .payload = frameBlock},
    MenuEntry{.label = "CameraBlock", .payload = cameraBlock},

    SEPARATOR,

    MenuEntry{.label = "Constants", .payload = builtInConstants},
    MenuEntry{
      .label = "Samplers",
      .isEnabled = fragmentShaderOnly,
      .payload = builtInSamplers,
    },
  };
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_WAREHOUSE, .label = "Built-ins", .payload = builtIns});

  // ---

  entries.emplace_back(SEPARATOR);

  const auto addArithmeticOperation = [](ArithmeticNode::Operation op) {
    return NodeFactoryEx{
      std::nullopt,
      [op](ShaderGraph &g, VertexDescriptor parent) {
        return ArithmeticNode::create(g, parent, op);
      },
    };
  };

  static const auto arithmetic = MenuEntries{
    MenuEntry{
      .icon = ICON_FA_PLUS,
      .label = "Add",
      .payload = addArithmeticOperation(ArithmeticNode::Operation::Add),
    },
    MenuEntry{
      .icon = ICON_FA_MINUS,
      .label = "Subtract",
      .payload = addArithmeticOperation(ArithmeticNode::Operation::Subtract),
    },
    MenuEntry{
      .icon = ICON_FA_XMARK,
      .label = "Multiply",
      .payload = addArithmeticOperation(ArithmeticNode::Operation::Multiply),
    },
    MenuEntry{
      .icon = ICON_FA_DIVIDE,
      .label = "Divide",
      .payload = addArithmeticOperation(ArithmeticNode::Operation::Divide),
    },
  };
  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_CALCULATOR, .label = "Arithmetic", .payload = arithmetic});

#define ADD_COMPOUND_NODE(Icon, T)                                             \
  MenuEntry {                                                                  \
    .icon = Icon, .label = #T, .payload = NodeFactoryEx {                      \
      std::nullopt, T##Node::create                                            \
    }                                                                          \
  }

  entries.emplace_back(ADD_COMPOUND_NODE(ICON_FA_LINK, Append));
  entries.emplace_back(ADD_COMPOUND_NODE(ICON_FA_CODE_BRANCH, Swizzle));
  entries.emplace_back(ADD_COMPOUND_NODE(ICON_FA_LINK_SLASH, VectorSplitter));
  entries.emplace_back(ADD_COMPOUND_NODE(ICON_FA_LINK_SLASH, MatrixSplitter));
  entries.emplace_back(ADD_COMPOUND_NODE(ICON_FA_ROTATE, MatrixTransform));

  if (!scriptedFunctions.empty()) {
    entries.emplace_back(SEPARATOR);
    insertScriptedFunctionEntries(entries, scriptedFunctions);
  }
  if (!userFunctions.empty()) {
    entries.emplace_back(SEPARATOR);
    entries.emplace_back(MenuEntry{
      .icon = ICON_FA_USER,
      .label = "User functions",
      .payload = buildUserFunctionEntries(userFunctions),
    });
  }

  entries.emplace_back(SEPARATOR);

  entries.emplace_back(MenuEntry{
    .icon = ICON_FA_IMAGE,
    .label = "Texture",
    .payload =
      NodeFactoryEx{
        std::nullopt,
        [](auto &, auto) { return TextureParam{}; },
      },
  });
  entries.emplace_back(ADD_COMPOUND_NODE(ICON_FA_EYE_DROPPER, TextureSampling));

  return entries;

#undef ADD_COMPOUND_NODE
#undef ADD_ENUM_VALUE
#undef SEPARATOR
}

std::optional<IDPair> processNodeMenu(
  ShaderGraph &g, std::span<const std::optional<MenuEntry>> entries,
  const std::string_view pattern, const rhi::ShaderType shaderType,
  const gfx::Material::Blueprint &blueprint) {
  if (entries.empty()) return std::nullopt;

  std::optional<IDPair> result;

  const auto matches = [pattern](const auto &entry) {
    if (!pattern.empty() && entry &&
        std::holds_alternative<NodeFactoryEx>(entry->payload) &&
        !entry->label.empty()) {
      return contains(entry->label.data(), pattern.data());
    }
    return true;
  };

  auto filteredEntries = entries | std::ranges::views::filter(matches);
  if (filteredEntries.empty()) {
    ImGui::MenuItem("(empty)", nullptr, nullptr, false);
  } else {
    for (const auto &menuEntry : filteredEntries) {
      if (!menuEntry) {
        if (pattern.empty()) {
          ImGui::Spacing();
          ImGui::Separator();
          ImGui::Spacing();
        }
        continue;
      }

      auto &[icon, label, callback, payload] = *menuEntry;
      const auto enabled = callback ? (*callback)(shaderType, blueprint) : true;

      std::visit(Overload{
                   [&](const NodeFactoryEx &p) {
                     const auto &[tooltip, factory] = p;
                     if (ImGui::MenuItemEx(label.data(), icon, nullptr, false,
                                           enabled)) {
                       result = createNode(g, factory);
                     }
                     if (tooltip && ImGui::IsKeyDown(ImGuiMod_Alt) &&
                         ImGui::IsItemHovered()) {
                       ImGui::ShowTooltip(tooltip->c_str());
                     }
                   },
                   [&](const MenuEntries &v) {
                     if (ImGui::BeginMenuEx(label.data(), icon, enabled)) {
                       result =
                         processNodeMenu(g, v, pattern, shaderType, blueprint);
                       ImGui::EndMenu();
                     }
                   },
                 },
                 payload);
    }
  }
  return result;
}
