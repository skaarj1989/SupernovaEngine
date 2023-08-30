#pragma once

#include "ShaderGraph.hpp"
#include "PathMap.hpp"
#include "renderer/Material.hpp"

#include "TextEditor.h"
#include "imnodes.h"

#include <expected>

struct MaterialProject {
  MaterialProject() = default;
  MaterialProject(const MaterialProject &) = delete;
  ~MaterialProject() = default;

  MaterialProject &operator=(const MaterialProject &) = delete;
  MaterialProject &operator=(MaterialProject &&) noexcept = default;

  operator bool() const;

  void init(gfx::MaterialDomain);
  void clear();

  bool save();
  bool saveAs(const std::filesystem::path &);

  bool exportMaterial(std::filesystem::path) const;

  struct Payload {
    PathMap texturePaths;
  };
  using LoadResult = std::expected<Payload, std::string>;
  [[nodiscard]] LoadResult load(const std::filesystem::path &);

  bool postLoad(gfx::TextureManager &, const PathMap &,
                const ScriptedFunctions &);

  void addUserFunction(UserFunctionData);
  // .first = Number of removed nodes (across all stages).
  // .second = true if material should be recompiled.
  std::pair<std::size_t, bool> removeUserFunction(const UserFunctionData *);

  bool isUsed(const UserFunctionData *) const;

  void setErrorMarkers(const std::map<rhi::ShaderType, std::string> &);
  void resetErrorMarkers();

  [[nodiscard]] std::expected<std::chrono::nanoseconds, std::string>
  composeMaterial();
  [[nodiscard]] gfx::Material buildMaterial();

  // ---

  std::optional<std::filesystem::path> path;

  std::string name;
  gfx::Material::Blueprint blueprint;

  UserFunctions userFunctions;

  struct Stage {
    Stage();
    Stage(const Stage &) = delete;
    Stage(Stage &&) noexcept = default;
    ~Stage() = default;

    Stage &operator=(const Stage &) = delete;
    Stage &operator=(Stage &&) noexcept = default;

    template <class Archive> void save(Archive &archive) const {
      archive(graph);

      const auto ini = std::string{
        ImNodes::SaveEditorStateToIniString(nodeEditorContext.get())};
      archive(ini);
    }
    template <class Archive> void load(Archive &archive) {
      archive(graph);

      std::string ini;
      archive(ini);
      ImNodes::LoadEditorStateFromIniString(nodeEditorContext.get(),
                                            ini.c_str(), ini.length());

      // Can't load TextureParam and GenericNode functions right here.
      // boost::adjacency_list is not movable.
    }

    ShaderGraph graph;

    struct EditorContextDeleter {
      void operator()(ImNodesEditorContext *ctx) const {
        ImNodes::EditorContextFree(ctx);
      }
    };
    using EditorContextPtr =
      std::unique_ptr<ImNodesEditorContext, EditorContextDeleter>;
    EditorContextPtr nodeEditorContext;
    TextEditor codeEditor;
  };

  using Stages = std::map<rhi::ShaderType, Stage>;
  Stages stages;
};
