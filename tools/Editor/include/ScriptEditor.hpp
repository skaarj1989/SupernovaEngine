#pragma once

#include "WidgetWindow.hpp"
#include "TextEditor.h"
#include "ScriptResource.hpp"
#include "entt/signal/emitter.hpp"
#include "sol/state.hpp"

class ScriptEditor final : public WidgetWindow,
                           private entt::emitter<ScriptEditor> {
  friend class entt::emitter<ScriptEditor>;

public:
  using entt::emitter<ScriptEditor>::on;

  ScriptEditor() = default;
  ScriptEditor(const ScriptEditor &) = delete;

  ScriptEditor &operator=(const ScriptEditor &) = delete;

  void openScript(std::shared_ptr<ScriptResource>);
  [[nodiscard]] bool hasScript(const std::shared_ptr<ScriptResource> &) const;

  void show(const char *name, bool *open) override;

  struct RunScriptRequest {
    std::string code;
  };

private:
  struct Entry {
    explicit Entry(std::shared_ptr<ScriptResource> = {});

    void save();
    void saveAs(const std::filesystem::path &p);

    bool hasResource() const;

    std::shared_ptr<ScriptResource> resource;

    TextEditor textEditor;
    bool dirty{false};
  };

  Entry *_getActiveEntry();
  void _removeScriptAt(std::size_t index);

private:
  std::vector<Entry> m_scripts;
  std::optional<std::size_t> m_activeScriptId;
};
