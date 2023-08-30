#include "MaterialEditor/MaterialProject.hpp"

#include "AlwaysFalse.hpp"
#include "TypeTraits.hpp"

#include "imnodes_internal.h" // context.Panning

#include "cereal/archives/binary.hpp"
#include "math/Serialization.hpp"
#include "cereal/types/utility.hpp" // pair
#include "cereal/types/memory.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/optional.hpp"
#include "cereal/types/variant.hpp"

#include "ShaderInfoLogParser.hpp"
#include "CreateNodeHelper.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include "MaterialExporter.hpp"

#include <fstream>

namespace gfx {

template <class Archive> void serialize(Archive &archive, DecalBlendMode &v) {
  archive(v.normal, v.emissive, v.albedo, v.metallicRoughnessAO);
}
template <class Archive>
void serialize(Archive &archive, Material::Surface &v) {
  archive(v.shadingModel, v.blendMode, v.decalBlendMode, v.lightingMode,
          v.cullMode);
}
template <class Archive>
void serialize(Archive &archive, Material::Blueprint &blueprint) {
  archive(blueprint.surface, blueprint.flags);
  // properties, defaultTextures and user*Code are stored in graph nodes.
}

} // namespace gfx

namespace {

constexpr char kMagicId[] = "sneMP";
constexpr auto kMagicIdSize = sizeof(kMagicId);

void buildTexturePaths(PathMap &map, const rhi::ShaderType shaderType,
                       const ShaderGraph &g) {
  for (const auto vd : g.vertices()) {
    const auto &prop = g.getVertexProp(vd);
    if (auto *param = std::get_if<TextureParam>(&prop.variant); param) {
      if (auto resource = std::dynamic_pointer_cast<Resource>(param->texture);
          resource) {
        map.addPath(shaderType, prop.id, resource->getPath());
      }
    }
  }
}

template <typename Func>
[[nodiscard]] auto findVertices(const ShaderGraph &g, Func f) {
  const auto match = [&g, &f](VertexDescriptor vd) {
    const auto &prop = g.getVertexProp(vd);
    if (bool(prop.flags & NodeFlags::Internal)) return false;
    return f(prop);
  };

  std::vector<VertexDescriptor> vertices;
  vertices.reserve(g.countVertices());
  std::ranges::copy(g.vertices() | std::ranges::views::filter(match),
                    std::back_inserter(vertices));
  return vertices;
}

[[nodiscard]] bool hasUserFunction(const VertexProp::Variant &v,
                                   const UserFunctionData *data) {
  if (auto *compoundNode = std::get_if<CompoundNodeVariant>(&v); compoundNode) {
    if (auto *customNode = std::get_if<CustomNode>(compoundNode)) {
      return customNode && customNode->data == data;
    }
  }
  return false;
}
[[nodiscard]] auto
findUserFunctions(const ShaderGraph &g,
                  std::span<const UserFunctionData *const> functions) {
  return findVertices(g, [functions](const VertexProp &prop) {
    return std::ranges::any_of(functions,
                               [&prop](const UserFunctionData *data) {
                                 return hasUserFunction(prop.variant, data);
                               });
  });
}

std::pair<std::size_t, bool> operator+(auto lhs, auto rhs) {
  return {lhs.first + rhs.first, lhs.second || rhs.second};
}
std::pair<std::size_t, bool> &operator+=(auto &lhs, auto rhs) {
  lhs = lhs + rhs;
  return lhs;
}

[[nodiscard]] auto removeNodesWithUserFunction(
  ShaderGraph &g, const std::set<VertexDescriptor> &connectedVertices,
  std::span<const UserFunctionData *const> functions) {
  auto dirty = false;

  const auto deleteList = findUserFunctions(g, functions);
  for (const auto vd : deleteList) {
    removeNode(g, vd);
    dirty |= connectedVertices.contains(vd);
  }
  return std::pair{deleteList.size(), dirty};
}

void scanDependencies(std::unordered_set<std::size_t> &dependencies,
                      std::size_t h, const UserFunctions &functions) {
  for (const auto &[hash, data] : functions) {
    if (const auto it = std::ranges::find(data->dependencies, h);
        it != data->dependencies.cend()) {
      dependencies.insert(hash);
      scanDependencies(dependencies, hash, functions);
    }
  }
}
[[nodiscard]] auto getDependencies(const UserFunctions &userFunctions,
                                   const UserFunctionData *data) {
  assert(data);
  const auto hash = std::hash<UserFunctionData>{}(*data);
  std::unordered_set<std::size_t> out{hash};
  scanDependencies(out, hash, userFunctions);
  return out;
}

void buildUserModules(std::ostringstream &oss,
                      const rhi::ShaderStages shaderStage,
                      std::pair<std::size_t, const UserFunctionData *> function,
                      std::unordered_set<std::size_t> &alreadyIncluded,
                      const UserFunctions &userFunctions) {
  auto [h, data] = function;
  if ((data->shaderStages & shaderStage) != shaderStage) return;
  if (alreadyIncluded.contains(h)) return;

  for (const auto dh : data->dependencies) {
    if (const auto it = userFunctions.find(dh); it != userFunctions.cend()) {
      buildUserModules(oss, shaderStage, {dh, it->second.get()},
                       alreadyIncluded, userFunctions);
    }
  }

  oss << buildDefinition(*data) << "\n";
  alreadyIncluded.insert(h);
}
[[nodiscard]] auto buildUserModules(const UserFunctions &userFunctions,
                                    const rhi::ShaderType shaderType) {
  std::ostringstream oss;
  std::unordered_set<std::size_t> alreadyIncluded;
  for (const auto &[h, data] : userFunctions) {
    buildUserModules(oss, getStage(shaderType), {h, data.get()},
                     alreadyIncluded, userFunctions);
  }
  return oss.str();
}

// --

decltype(auto) printOrder(std::ostream &os, const ShaderGraph &g,
                          std::stack<VertexDescriptor> s) {
  while (!s.empty()) {
    const auto c = s.top();
    s.pop();

    const auto &v = g.getVertexProp(c);
    std::ostream_iterator<std::string>{os, "\n"} = v.toString();
  }
  return os;
}

[[nodiscard]] std::expected<std::string, std::string>
evaluate(MaterialGenerationContext &context, const ShaderGraph &g,
         const rhi::ShaderType shaderType) {
  MaterialGenerationContext::Shader shader{.type = shaderType};
  context.currentShader = &shader;

  auto nodes = g.getExecutionOrder(g.getRoot()->vd);
  // printOrder(std::cout, g, nodes);

  const auto passthrough = [&] {
    return extractTop(context.currentShader->tokens);
  };

  while (!nodes.empty()) {
    const auto vd = nodes.top();
    nodes.pop();

    const auto &vertexProp = g.getVertexProp(vd);
    const auto id = vertexProp.id;
    assert(id >= 0);

    // Some nodes don't return a value (MasterNode or ContainerNode), hence
    // the monostate.
    using Result = std::variant<std::monostate, NodeResult>;

    auto result = std::visit(
      [&](const auto &arg) -> Result {
        // An internal value of a node will be used when there is no
        // outgoing edge.
        const auto useInternalValue = g.countOutEdges(vd) == 0;

        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::monostate>) {
          return useInternalValue ? Result{std::unexpected{"Undefined type."}}
                                  : Result{passthrough()};
        } else if constexpr (std::is_same_v<T, ContainerNode>) {
          return std::monostate{}; // Nothing to do.
        } else if constexpr (is_any_v<T, ValueVariant, Attribute,
                                      BuiltInConstant, BuiltInSampler>) {
          return useInternalValue ? Result{evaluate(context, id, arg)}
                                  : Result{passthrough()};
        } else if constexpr (is_any_v<T, FrameBlockMember, CameraBlockMember,
                                      SplitVector, SplitMatrix>) {
          return evaluate(context, id, arg);
        } else if constexpr (is_any_v<T, PropertyValue, TextureParam>) {
          return useInternalValue
                   ? Result{evaluate(context, id, getUserLabel(vertexProp),
                                     arg)}
                   : Result{passthrough()};
        } else if constexpr (std::is_same_v<T, CompoundNodeVariant>) {
          return std::visit(
            [&](auto &node) { return node.evaluate(context, id); }, arg);
        } else if constexpr (std::is_same_v<T, MasterNodeVariant>) {
          auto result = std::visit(
            [&](auto &master) { return master.evaluate(context, id); }, arg);
          return result ? Result{} : Result{std::unexpected{result.error()}};
        } else {
          static_assert(always_false_v<T>, "non-exhaustive visitor!");
        }
      },
      vertexProp.variant);

    if (auto nodeResult = std::get_if<NodeResult>(&result); nodeResult) {
      if (nodeResult->has_value()) {
        shader.tokens.push(nodeResult->value());
      } else {
        return std::unexpected{
          std::format("[ID: {}] {}", id, nodeResult->error()),
        };
      }
    }
  }
  return shader.composer.getCode();
}

void copyProperties(const MaterialGenerationContext &context,
                    std::vector<gfx::Property> &properties) {
  properties.clear();
  properties.reserve(context.properties.size());
  for (const auto &[name, value] : context.properties)
    properties.emplace_back(name, value);
}

[[nodiscard]] auto makeTextureInfo(std::shared_ptr<rhi::Texture> texture) {
  return gfx::TextureInfo{
    .type = texture ? texture->getType() : rhi::TextureType::Undefined,
    .texture = texture,
  };
}
void copyTextures(const MaterialGenerationContext &context,
                  gfx::TextureResources &textures) {
  textures.clear();
  std::ranges::transform(context.textures,
                         std::inserter(textures, textures.end()),
                         [](const auto &p) {
                           return std::pair{p.first, makeTextureInfo(p.second)};
                         });
}

gfx::Material::Blueprint::Code *
getCodeSection(gfx::Material::Blueprint &blueprint,
               const rhi::ShaderType shaderType) {
  switch (shaderType) {
    using enum rhi::ShaderType;

  case Vertex:
    return &blueprint.userVertCode;
  case Fragment:
    return &blueprint.userFragCode;
  }
  return nullptr;
}

[[nodiscard]] auto getPointers(const UserFunctions &userFunctions,
                               const auto &container) {
  std::vector<const UserFunctionData *> result(container.size());
  std::ranges::transform(
    container, result.begin(), [&userFunctions](auto hash) {
      const auto it = userFunctions.find(hash);
      return it != userFunctions.cend() ? it->second.get() : nullptr;
    });
  return result;
}

} // namespace

//
// MaterialProject::Stage struct:
//

using Stage = MaterialProject::Stage;

Stage::Stage() {
  nodeEditorContext.reset(ImNodes::EditorContextCreate());
  nodeEditorContext->Panning = {600, 100};
  codeEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
}

//
// MaterialProject class:
//

MaterialProject::operator bool() const { return !stages.empty(); }

void MaterialProject::init(gfx::MaterialDomain domain) {
  clear();

  switch (domain) {
    using enum gfx::MaterialDomain;

  case Surface:
    blueprint.surface.emplace();
    blueprint.flags = gfx::MaterialFlags::SurfaceDefault;
    break;
  case PostProcess:
    blueprint.flags = gfx::MaterialFlags::Enabled;
    break;
  }

  using enum rhi::ShaderType;
  if (blueprint.surface) {
    auto &vertexStage = stages[Vertex];
    createNode(vertexStage.graph, VertexMasterNode::create);
    auto &fragmentStage = stages[Fragment];
    createNode(fragmentStage.graph, SurfaceMasterNode::create);
  } else {
    auto &fragmentStage = stages[Fragment];
    createNode(fragmentStage.graph, PostProcessMasterNode::create);
  }
}
void MaterialProject::clear() {
  path = std::nullopt;
  name.clear();
  blueprint = {};
  userFunctions.clear();
  stages.clear();
}

bool MaterialProject::save() { return path ? saveAs(*path) : false; }
bool MaterialProject::saveAs(const std::filesystem::path &p) {
  try {
    std::ofstream f{p, std::ios::binary};
    {
      cereal::BinaryOutputArchive archive{f};
      archive.saveBinary(kMagicId, kMagicIdSize);

      archive(name, blueprint, userFunctions);

      PathMap texturePaths{p.parent_path()};
      for (const auto &[shaderType, stage] : stages) {
        buildTexturePaths(texturePaths, shaderType, stage.graph);
      }
      archive(texturePaths, stages);
    }
  } catch (const std::exception &e) {
    return false;
  }
  path = p;
  return true;
}

bool MaterialProject::exportMaterial(std::filesystem::path p) const {
  return ::exportMaterial(p.replace_extension(".material"), name, blueprint);
}

MaterialProject::LoadResult
MaterialProject::load(const std::filesystem::path &p) {
  Payload payload{.texturePaths = PathMap{p.parent_path()}};
  try {
    std::ifstream f{p, std::ios::binary};
    {
      cereal::BinaryInputArchive archive{f};
      std::remove_const_t<decltype(kMagicId)> magicId{};
      archive.loadBinary(magicId, kMagicIdSize);
      if (strncmp(magicId, kMagicId, kMagicIdSize) != 0)
        return std::unexpected{"Invalid MaterialProject signature."};

      archive(name, blueprint, userFunctions, payload.texturePaths, stages);
    }
  } catch (const std::exception &e) {
    return std::unexpected{e.what()};
  }

  path = p;
  return payload;
}

bool MaterialProject::postLoad(gfx::TextureManager &textureManager,
                               const PathMap &texturePaths,
                               const ScriptedFunctions &scriptedFunctions) {
  for (auto &[shaderType, stage] : stages) {
    auto &g = stage.graph;
    for (const auto vd : g.vertices()) {
      auto &vertexProp = g.getVertexProp(vd);
      const auto loaded = std::visit(
        [&](auto &arg) {
          using T = std::decay_t<decltype(arg)>;

          if constexpr (std::is_enum_v<T> ||
                        is_any_v<T, std::monostate, ContainerNode, ValueVariant,
                                 PropertyValue, MasterNodeVariant>) {
            // Nothing to do.
          } else if constexpr (std::is_same_v<T, TextureParam>) {
            if (const auto p = texturePaths.getPath(shaderType, vertexProp.id);
                p) {
              arg.texture = textureManager.load(*p).handle();
            }
          } else if constexpr (std::is_same_v<T, CompoundNodeVariant>) {
            if (auto scriptedNode = std::get_if<ScriptedNode>(&arg);
                scriptedNode) {
              // Loading should not fail if a script could not be found.
              scriptedNode->setFunction(scriptedFunctions);
            } else if (auto customNode = std::get_if<CustomNode>(&arg);
                       customNode) {
              // This should never fail.
              // An user function is saved in a a project file.
              return customNode->setFunction(userFunctions);
            }
          } else {
            static_assert(always_false_v<T>, "non-exhaustive visitor!");
          }
          return true;
        },
        vertexProp.variant);
      if (!loaded) return false;
    }
  }
  return true;
}

void MaterialProject::addUserFunction(UserFunctionData data) {
  const auto hash = std::hash<UserFunctionData>{}(data);
  userFunctions.try_emplace(
    hash, std::make_unique<UserFunctionData>(std::move(data)));
}
std::pair<std::size_t, bool>
MaterialProject::removeUserFunction(const UserFunctionData *data) {
  const auto removeList = getDependencies(userFunctions, data);
  const auto functions = getPointers(userFunctions, removeList);

  std::pair<std::size_t, bool> result;
  for (auto &[_, stage] : stages) {
    const auto linked = findConnectedVertices(stage.graph);
    result += removeNodesWithUserFunction(stage.graph, linked, functions);
  }
  for (const auto h : removeList) {
    userFunctions.erase(h);
  }
  return result;
}

bool MaterialProject::isUsed(const UserFunctionData *data) const {
  const auto dependencies = getDependencies(userFunctions, data);
  const auto functions = getPointers(userFunctions, dependencies);

  for (const auto &[_, stage] : stages) {
    const auto vertices = findUserFunctions(stage.graph, functions);
    if (std::ranges::any_of(
          vertices, [linked = findConnectedVertices(stage.graph)](
                      VertexDescriptor vd) { return linked.contains(vd); })) {
      return true;
    }
  }
  return false;
}

void MaterialProject::setErrorMarkers(
  const std::map<rhi::ShaderType, std::string> &stageError) {
  for (const auto &[shaderType, infoLog] : stageError) {
    auto &editor = stages[shaderType].codeEditor;
    auto errorMarkers = getErrorMarkers(infoLog);
    auto lastLine = 0;
    for (const auto &[line, _] : errorMarkers) {
      lastLine = std::max(lastLine, line);
    }
    // "unexpected RIGHT_BRACE, expecting RIGHT_PAREN" is in line n+1.
    if (lastLine > editor.GetTotalLines()) editor.InsertText("\n");
    editor.SetErrorMarkers(errorMarkers);
  }
}
void MaterialProject::resetErrorMarkers() {
  for (auto &[_, stage] : stages)
    stage.codeEditor.SetErrorMarkers({});
}

std::expected<std::chrono::nanoseconds, std::string>
MaterialProject::composeMaterial() {
  const auto begin = std::chrono::steady_clock::now();

  MaterialGenerationContext context{
    .materialDomain = getDomain(blueprint),
  };

  for (auto &[shaderType, stage] : stages) {
    if (auto result = evaluate(context, stage.graph, shaderType); result) {
      getCodeSection(blueprint, shaderType)->includes =
        buildUserModules(userFunctions, shaderType);

      stage.codeEditor.SetText(*result);
      stage.codeEditor.SetErrorMarkers({});
    } else {
      return std::unexpected{result.error()};
    }
  }
  copyProperties(context, blueprint.properties);
  copyTextures(context, blueprint.defaultTextures);

  return (std::chrono::steady_clock::now() - begin);
}

gfx::Material MaterialProject::buildMaterial() {
#define COPY_CODE(Stage, code)                                                 \
  if (auto it = stages.find(rhi::ShaderType::Stage); it != stages.cend()) {    \
    blueprint.code.source = it->second.codeEditor.GetText();                   \
  }

  COPY_CODE(Vertex, userVertCode)
  COPY_CODE(Fragment, userFragCode)

  return gfx::Material::Builder{}
    .setName(name)
    .setDomain(gfx::getDomain(blueprint))
    .setBlueprint(blueprint)
    .build();
}
