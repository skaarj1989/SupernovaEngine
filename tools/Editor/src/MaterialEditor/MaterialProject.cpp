#include "MaterialEditor/MaterialProject.hpp"

#include "cereal/archives/binary.hpp"

#include "math/Serialization.hpp"
#include "os/Serialization.hpp"
#include "cereal/types/utility.hpp" // pair
#include "cereal/types/memory.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/optional.hpp"
#include "cereal/types/variant.hpp"

#include "MaterialEditor/TextureParam.hpp"
#include "MaterialEditor/Nodes/Embedded.hpp"
#include "MaterialEditor/Nodes/Custom.hpp"

#include "MaterialEditor/Nodes/VertexMaster.hpp"
#include "MaterialEditor/Nodes/SurfaceMaster.hpp"
#include "MaterialEditor/Nodes/PostProcessMaster.hpp"

#include "MaterialExporter.hpp"
#include "MaterialEditor/ShaderCodeEvaluator.hpp"

#include "renderer/WorldRenderer.hpp"

#include "imnodes_internal.h" // context.Panning

#include "tracy/Tracy.hpp"

#include <bitset>
#include <ranges>
#include <format>
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

[[nodiscard]] auto countStages(const rhi::ShaderStages flags) {
  return std::bitset<sizeof(rhi::ShaderStages) * 8>{std::to_underlying(flags)}
    .count();
}

template <typename Func>
concept NodeFilter = std::predicate<Func, const NodeBase *>;

template <NodeFilter Func>
[[nodiscard]] auto findVertices(const ShaderGraph &g, Func f) {
  const auto match = [&f](const NodeBase *node) {
    return nonInternalNode(node) && f(node);
  };
  auto filteredNodes = g.nodes() | std::views::filter(match);

  std::vector<VertexDescriptor> vertices;
  vertices.reserve(std::ranges::distance(filteredNodes));
  std::ranges::transform(filteredNodes, std::back_inserter(vertices),
                         [](const NodeBase *node) { return node->vertex.vd; });
  return vertices;
}
[[nodiscard]] auto
findUserFunctionNodes(const ShaderGraph &g,
                      const std::unordered_set<UserFunction::ID> &functionIDs) {
  return findVertices(g, [functionIDs](const NodeBase *node) {
    const auto *customNode = dynamic_cast<const CustomNode *>(node);
    return customNode && functionIDs.contains(customNode->userFunction.id);
  });
}

void collectDependencies(const UserFunctions &functions,
                         std::span<const UserFunction::ID> targetFunctionIds,
                         std::unordered_set<UserFunction::ID> &out) {
  std::stack<UserFunction::ID> stack;
  std::unordered_set<UserFunction::ID> visited;
  for (const auto id : targetFunctionIds) {
    out.insert(id);
    stack.push(id);
  }
  while (!stack.empty()) {
    const auto currentFunctionId = stack.top();
    stack.pop();

    if (!visited.contains(currentFunctionId)) {
      visited.insert(currentFunctionId);
      for (const auto id : functions.at(currentFunctionId)->dependencies) {
        out.insert(id);
        stack.push(id);
      }
    }
  }
}

void copyProperties(const ShaderDefinition::PropertyMap &in,
                    const rhi::ShaderType stage,
                    std::vector<gfx::Property> &out) {
  std::erase_if(out, [prefix = getPrefix(stage)](const auto &p) {
    return p.name.starts_with(prefix);
  });
  for (const auto &[name, value] : in) {
    out.emplace_back(name, value);
  }
}

[[nodiscard]] auto makeTextureInfo(std::shared_ptr<rhi::Texture> texture) {
  return gfx::TextureInfo{
    .type = texture ? texture->getType() : rhi::TextureType::Undefined,
    .texture = texture,
  };
}
void copyTextures(const ShaderDefinition::TextureMap &in,
                  const rhi::ShaderType stage, gfx::TextureResources &out) {
  std::erase_if(out, [prefix = std::format("t_{}", getPrefix(stage))](
                       const auto &p) { return p.first.starts_with(prefix); });
  std::ranges::transform(in, std::inserter(out, out.end()), [](const auto &p) {
    return std::pair{p.first, makeTextureInfo(p.second)};
  });
}

} // namespace

//
// MaterialProject::Stage struct:
//

MaterialProject::Stage::Stage(const rhi::ShaderType shaderType)
    : graph{shaderType} {
  nodeEditorContext.reset(ImNodes::EditorContextCreate());
  nodeEditorContext->Panning = {600, 100};

  codeEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Glsl);
  codeEditor.SetPalette(TextEditor::PaletteId::Dark);
  codeEditor.SetShortTabsEnabled(true);
  codeEditor.SetShowWhitespacesEnabled(false);
  codeEditor.SetTabSize(2);
}

void MaterialProject::Stage::EditorContextDeleter::operator()(
  ImNodesEditorContext *ctx) const {
  ImNodes::EditorContextFree(ctx);
}

template <class Archive>
void save(Archive &archive, const MaterialProject::Stage &stage) {
  archive(stage.graph, std::string{ImNodes::SaveEditorStateToIniString(
                         stage.nodeEditorContext.get())});
}
template <class Archive>
void load(Archive &archive, MaterialProject::Stage &stage) {
  std::string ini;
  archive(stage.graph, ini);
  ImNodes::LoadEditorStateFromIniString(stage.nodeEditorContext.get(),
                                        ini.data(), ini.length());
}

//
// MaterialProject class:
//

MaterialProject::MaterialProject(const ScriptedFunctions *scriptedFunctions)
    : m_patcher{&m_pathMap, scriptedFunctions, &m_userFunctions} {
  m_stages.reserve(kSupportedStages.size());
}

MaterialProject::operator bool() const { return !m_stages.empty(); }
bool MaterialProject::operator==(const std::filesystem::path &p) const {
  return m_path == p;
}

MaterialProject &MaterialProject::init(const gfx::MaterialDomain domain) {
  clear();
  switch (domain) {
    using enum gfx::MaterialDomain;
    using enum rhi::ShaderType;

  case Surface:
    m_blueprint.surface.emplace();
    m_blueprint.flags = gfx::MaterialFlags::SurfaceDefault;
    _addStage(Vertex)->graph.add<VertexMasterNode>();
    _addStage(Fragment)->graph.add<SurfaceMasterNode>();
    break;
  case PostProcess:
    m_blueprint.flags = gfx::MaterialFlags::Enabled;
    _addStage(Fragment)->graph.add<PostProcessMasterNode>();
    break;
  }
  publish(MaterialProjectInitializedEvent{});
  return *this;
}
MaterialProject &MaterialProject::clear() {
  if (!m_stages.empty()) {
    publish(BeforeMaterialProjectUnloadEvent{});

    CommandInvoker::clear();
    m_name.clear();
    m_blueprint = {};

    m_path = std::nullopt;
    m_savedHistoryIndex = -1;

    m_pathMap.reset();
    m_userFunctions.clear();
    m_stages.clear();
    m_dirtyStages = rhi::ShaderStages{};
  }
  return *this;
}

MaterialProject::SaveResult MaterialProject::save() {
  return m_path ? saveAs(*m_path) : "Path not specified.";
}
MaterialProject::SaveResult
MaterialProject::saveAs(const std::filesystem::path &p) {
  try {
    std::ofstream f{p, std::ios::binary | std::ios::trunc};
    {
      cereal::BinaryOutputArchive archive{f};
      archive.saveBinary(kMagicId, kMagicIdSize);

      // clang-format off
      archive(
        m_name,
        m_blueprint,
        m_userFunctions,
        makeRelative(m_pathMap.getStorage(), p.parent_path()),
        statActiveStages()
      );
      // clang-format on
      for (const auto &stage : m_stages) {
        archive(stage->graph.getShaderType(), *stage);
      }
    }
  } catch (const std::exception &e) {
    return e.what();
  }
  m_path = p;
  m_savedHistoryIndex = getHistoryIndex();

  return std::nullopt;
}

MaterialProject::LoadResult
MaterialProject::load(const std::filesystem::path &p) {
  clear();
  try {
    std::ifstream f{p, std::ios::binary};
    cereal::BinaryInputArchive archive{f};

    std::remove_const_t<decltype(kMagicId)> magicId{};
    archive.loadBinary(magicId, kMagicIdSize);
    if (strncmp(magicId, kMagicId, kMagicIdSize) != 0) {
      return "Invalid MaterialProject signature.";
    }

    rhi::ShaderStages usedStages;
    PathMap::Storage pathStorage;
    // clang-format off
    archive(
      m_name,
      m_blueprint,
      m_userFunctions,
      pathStorage,
      usedStages
    );
    // clang-format on
    m_pathMap.reset(makeAbsolute(std::move(pathStorage), p.parent_path()));

    for (auto i = 0; i < countStages(usedStages); ++i) {
      rhi::ShaderType type{};
      archive(type);
      auto *stage = _addStage(type);
      archive(*stage);
    }
  } catch (const std::exception &e) {
    return e.what();
  }
  publish(MaterialProjectInitializedEvent{});

  m_path = p;
  return std::nullopt;
}

bool MaterialProject::isOnDisk() const { return m_path.has_value(); }
bool MaterialProject::isChanged() const {
  return getHistoryIndex() != m_savedHistoryIndex;
}

MaterialProject::SaveResult MaterialProject::exportMaterial() const {
  return m_path ? exportMaterial(*m_path) : "Path not specified.";
}
MaterialProject::SaveResult
MaterialProject::exportMaterial(std::filesystem::path p) const {
  return ::exportMaterial(p.replace_extension(".material"), m_name, m_blueprint)
           ? std::nullopt
           : std::make_optional("Could not open file.");
}

std::optional<UserFunction::ID>
MaterialProject::addUserFunction(UserFunction::Data &&data) {
  const auto functionId = std::hash<UserFunction::Data>{}(data);
  const auto [_, inserted] = m_userFunctions.try_emplace(
    functionId, std::make_unique<UserFunction::Data>(std::move(data)));
  if (inserted) {
    publish(UserFunctionAddedEvent{.id = functionId});
    return functionId;
  } else {
    return std::nullopt;
  }
}
std::optional<std::string>
MaterialProject::updateUserFunction(const UserFunction::ID functionId,
                                    std::string code) {
  auto it = m_userFunctions.find(functionId);
  if (it == m_userFunctions.cend() || it->second->code == code)
    return std::nullopt;

  std::swap(it->second->code, code);
  publish(UserFunctionUpdatedEvent{.id = functionId});

  const auto functionList = _getReverseDependencyList(functionId);
  for (const auto &stage : m_stages) {
    const auto vertices = findUserFunctionNodes(stage->graph, functionList);
    for (const auto vd : vertices) {
      stage->graph.getNode(vd)->markDirty();
    }
  }
  return code;
}
bool MaterialProject::removeUserFunction(const UserFunction::ID id) {
  if (m_userFunctions.erase(id) > 0) {
    publish(UserFunctionRemovedEvent{.id = id});
    return true;
  }
  return false;
}

MaterialProject::UserFunctionScope
MaterialProject::getUserFunctionScope(const UserFunction::ID id) const {
  UserFunctionScope result{.functionList = _getReverseDependencyList(id)};
  for (const auto &stage : m_stages) {
    auto &g = stage->graph;
    UserFunctionScope::SubGraph subGraph{
      .vertices = findUserFunctionNodes(g, result.functionList),
    };
    for (const auto vd : subGraph.vertices) {
      collectExternalEdges(g, vd, subGraph.edges);
    }
    result.stages[&g] = std::move(subGraph);
  }
  return result;
}

bool MaterialProject::hasUserFunctions() const {
  return !m_userFunctions.empty();
}
const UserFunctions &MaterialProject::getUserFunctions() const {
  return m_userFunctions;
}
const UserFunction::Data *
MaterialProject::getUserFunctionData(const UserFunction::ID id) const {
  if (const auto it = m_userFunctions.find(id); it != m_userFunctions.cend()) {
    return it->second.get();
  }
  return nullptr;
}

std::optional<MaterialProject::ErrorMessage>
MaterialProject::compose(const rhi::ShaderStages stages) {
  for (const auto shaderType : kSupportedStages) {
    if (bool(stages & rhi::getStage(shaderType)))
      if (auto error = _compose(shaderType); error) return error;
  }
  publish(ShaderComposedEvent{.stages = stages});
  return std::nullopt;
}

bool MaterialProject::buildMaterial(const gfx::WorldRenderer &renderer) {
  ZoneScopedN("BuildMaterial");

  for (const auto &stage : m_stages) {
    m_blueprint.userCode[stage->graph.getShaderType()].source =
      stage->codeEditor.GetText();
  }

  auto material =
    gfx::Material::Builder{}.setName(m_name).setBlueprint(m_blueprint).build();
  if (auto error = renderer.isValid(material); !error) {
    publish(MaterialBuiltEvent{
      .material = std::make_unique<gfx::Material>(std::move(material)),
    });
    return true;
  }
  return false;
}

const gfx::Material::Blueprint &MaterialProject::getBlueprint() const {
  return m_blueprint;
}
gfx::MaterialDomain MaterialProject::getMaterialDomain() const {
  return gfx::getDomain(m_blueprint);
}

MaterialProject::Stage *
MaterialProject::getStage(const rhi::ShaderType shaderType) {
  for (auto &stage : m_stages) {
    if (stage->graph.getShaderType() == shaderType) return stage.get();
  }
  return nullptr;
}

rhi::ShaderStages MaterialProject::statActiveStages() const {
  rhi::ShaderStages activeStages{};
  for (const auto &stage : m_stages) {
    activeStages |= rhi::getStage(stage->graph.getShaderType());
  }
  return activeStages;
}

rhi::ShaderStages MaterialProject::getDirtyStages() const {
  return m_dirtyStages;
}
bool MaterialProject::hasDirtyStages() const {
  return m_dirtyStages != rhi::ShaderStages{};
}

//
// (private):
//

MaterialProject::Stage *
MaterialProject::_addStage(const rhi::ShaderType shaderType) {
  assert(getStage(shaderType) == nullptr);
  auto &stage = m_stages.emplace_back(std::make_unique<Stage>(shaderType));
  _connect(stage->graph);
  return stage.get();
}
void MaterialProject::_connect(ShaderGraph &g) {
  g.on<NodeAddedEvent>([this](const NodeAddedEvent &evt, const auto &) {
    evt.node->on<NodeValueUpdatedEvent>([this](const auto &, NodeBase &node) {
      if (auto *etn = dynamic_cast<EmbeddedNode<TextureParam> *>(&node); etn) {
        auto resource =
          std::dynamic_pointer_cast<const Resource>(etn->value.texture);
        if (resource) {
          m_pathMap.add(node.getOrigin(), node.vertex.id, resource->getPath());
        }
      }
      _checkMasterNodeCommit(node);
    });
    evt.node->on<NodeLabelUpdatedEvent>(
      [this](const auto &, const NodeBase &node) {
        _checkMasterNodeCommit(node);
      });

    m_patcher.patch(*evt.node);
  });

  const auto linkCheck = [this](const auto &evt, const auto &) {
    _checkMasterNodeCommit(*evt.source);
  };
  g.on<EdgeAddedEvent>(linkCheck);
  g.on<BeforeEdgeRemoveEvent>(linkCheck);
}

void MaterialProject::_checkMasterNodeCommit(const NodeBase &node) {
  const auto stage = rhi::getStage(node.getOrigin());
  if (!bool(m_dirtyStages & stage) && node.isLinkedToRoot()) {
    m_dirtyStages |= stage;
    publish(MasterNodeChangedEvent{.stages = stage});
  }
}

std::optional<MaterialProject::ErrorMessage>
MaterialProject::_compose(const rhi::ShaderType shaderType) {
  if (auto *stage = getStage(shaderType); stage) {
    m_dirtyStages &= ~rhi::getStage(shaderType);

    ShaderCodeEvaluator evaluator{getMaterialDomain()};
    if (auto result = evaluator.evaluate(stage->graph); result) {
      stage->codeEditor.SetText(result->code);

      m_blueprint.userCode[shaderType].includes =
        _buildUserModules(result->usedUserFunctions);

      copyProperties(result->properties, shaderType, m_blueprint.properties);
      copyTextures(result->textures, shaderType, m_blueprint.defaultTextures);
    } else {
      return result.error();
    }
  }
  return std::nullopt;
}

std::string MaterialProject::_buildUserModules(
  std::span<const UserFunction::ID> usedFunctionIds) const {
  std::unordered_set<UserFunction::ID> completeFunctionList;
  collectDependencies(m_userFunctions, usedFunctionIds, completeFunctionList);
  if (completeFunctionList.empty()) return "";

  std::ostringstream declarations;
  std::ostringstream definitions;

  for (const auto id : completeFunctionList) {
    const auto *data = getUserFunctionData(id);
    declarations << buildDeclaration(*data) << ";\n";
    definitions << buildDefinition(*data) << "\n";
  }
  return std::format("{}\n{}", declarations.str(), definitions.str());
}

std::unordered_set<UserFunction::ID>
MaterialProject::_getReverseDependencyList(const UserFunction::ID id) const {
  std::unordered_set<UserFunction::ID> out;

  std::stack<UserFunction::ID> stack;
  stack.push(id);

  while (!stack.empty()) {
    const auto currentFunctionId = stack.top();
    stack.pop();

    out.insert(currentFunctionId);
    for (const auto &[id, data] : m_userFunctions) {
      if (std::ranges::find(data->dependencies, currentFunctionId) !=
          data->dependencies.cend()) {
        stack.push(id);
      }
    }
  }
  return out;
}
