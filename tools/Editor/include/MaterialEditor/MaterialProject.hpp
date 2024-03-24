#pragma once

#include "ShaderGraph.hpp"
#include "ShaderGraphStageView.hpp"
#include "UserFunction.hpp"
#include "ScriptedFunction.hpp"
#include "Command.hpp"
#include "CommandInvoker.hpp"
#include "NodePatcherVisitor.hpp"
#include "PathMap.hpp"
#include "renderer/Material.hpp"

#include "TextEditor.h"

namespace gfx {
class WorldRenderer;
};

struct MaterialProjectInitializedEvent {};
struct BeforeMaterialProjectUnloadEvent {};
struct MasterNodeChangedEvent {
  rhi::ShaderStages stages{};
};
struct ShaderComposedEvent {
  rhi::ShaderStages stages{};
};
struct MaterialBuiltEvent {
  std::shared_ptr<gfx::Material> material;
};

struct UserFunctionAddedEvent {
  UserFunction::ID id;
};
struct UserFunctionUpdatedEvent {
  UserFunction::ID id;
};
struct UserFunctionRemovedEvent {
  UserFunction::ID id;
};

struct ImNodesEditorContext;

class MaterialProject : public CommandInvoker<Command>,
                        private entt::emitter<MaterialProject> {
  friend class entt::emitter<MaterialProject>;
  friend class MaterialEditor;

  static constexpr auto kSupportedStages = std::array{
    rhi::ShaderType::Vertex,
    rhi::ShaderType::Fragment,
  };

public:
  MaterialProject(const ScriptedFunctions *);
  MaterialProject(const MaterialProject &) = delete;
  ~MaterialProject() = default;

  MaterialProject &operator=(const MaterialProject &) = delete;
  MaterialProject &operator=(MaterialProject &&) noexcept = default;

  using entt::emitter<MaterialProject>::emitter;

  using entt::emitter<MaterialProject>::on;
  using entt::emitter<MaterialProject>::erase;

  operator bool() const;
  bool operator==(const std::filesystem::path &) const;

  MaterialProject &init(const gfx::MaterialDomain);
  MaterialProject &clear();

  using ErrorMessage = std::string;

  using SaveResult = std::optional<ErrorMessage>;
  SaveResult save();
  SaveResult saveAs(const std::filesystem::path &);

  using LoadResult = std::optional<ErrorMessage>;
  [[nodiscard]] LoadResult load(const std::filesystem::path &);

  [[nodiscard]] bool isOnDisk() const;
  [[nodiscard]] bool isChanged() const;

  SaveResult exportMaterial() const;
  SaveResult exportMaterial(std::filesystem::path) const;

  // ---

  std::optional<UserFunction::ID> addUserFunction(UserFunction::Data &&);
  std::optional<std::string> updateUserFunction(const UserFunction::ID,
                                                std::string code);
  bool removeUserFunction(const UserFunction::ID);

  struct UserFunctionScope {
    std::unordered_set<UserFunction::ID> functionList;
    struct SubGraph {
      std::vector<VertexDescriptor> vertices;
      std::unordered_set<EdgeDescriptor> edges;
    };
    std::unordered_map<ShaderGraph *, SubGraph> stages;
  };
  UserFunctionScope getUserFunctionScope(const UserFunction::ID) const;

  [[nodiscard]] const UserFunctions &getUserFunctions() const;
  [[nodiscard]] bool hasUserFunctions() const;
  [[nodiscard]] const UserFunction::Data *
  getUserFunctionData(const UserFunction::ID) const;

  // ---

  std::optional<ErrorMessage> compose(const rhi::ShaderStages);

  bool buildMaterial(const gfx::WorldRenderer &);

  const gfx::Material::Blueprint &getBlueprint() const;
  [[nodiscard]] gfx::MaterialDomain getMaterialDomain() const;

  struct Stage {
    explicit Stage(const rhi::ShaderType);
    Stage(const Stage &) = delete;
    Stage(Stage &&) noexcept = default;
    ~Stage() = default;

    Stage &operator=(const Stage &) = delete;
    Stage &operator=(Stage &&) noexcept = default;

    // ---

    ShaderGraph graph;

    struct EditorContextDeleter {
      void operator()(ImNodesEditorContext *) const;
    };
    using EditorContextPtr =
      std::unique_ptr<ImNodesEditorContext, EditorContextDeleter>;
    EditorContextPtr nodeEditorContext;
    TextEditor codeEditor;
  };

  Stage *getStage(const rhi::ShaderType);
  rhi::ShaderStages statActiveStages() const;

  rhi::ShaderStages getDirtyStages() const;
  bool hasDirtyStages() const;

private:
  Stage *_addStage(const rhi::ShaderType);
  void _connect(ShaderGraph &);

  void _checkMasterNodeCommit(const NodeBase &);

  std::optional<ErrorMessage> _compose(const rhi::ShaderType);

  std::string _buildUserModules(std::span<const UserFunction::ID>) const;

  // @return A list of function IDs (including the given one), that are
  // dependent on the provided function.
  [[nodiscard]] std::unordered_set<UserFunction::ID>
  _getReverseDependencyList(const UserFunction::ID) const;

private:
  std::string m_name;
  gfx::Material::Blueprint m_blueprint;

  std::optional<std::filesystem::path> m_path;
  int32_t m_savedHistoryIndex{-1};

  PathMap m_pathMap;
  UserFunctions m_userFunctions;
  NodePatcherVisitor m_patcher;

  std::vector<std::unique_ptr<Stage>> m_stages;
  rhi::ShaderStages m_dirtyStages{};
};

[[nodiscard]] inline auto makeView(MaterialProject::Stage &stage) {
  return ShaderGraphStageViewMutable{
    .graph = &stage.graph,
    .nodeEditorContext = stage.nodeEditorContext.get(),
  };
}
