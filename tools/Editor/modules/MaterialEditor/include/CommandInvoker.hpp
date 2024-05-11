#pragma once

#include <memory>
#include <vector>

template <class Command> class CommandInvoker {
public:
  template <class CommandT, typename... Args>
    requires std::is_base_of_v<Command, CommandT>
  CommandInvoker &addCommand(Args &&...args) {
    m_commands.push_back(
      std::make_unique<CommandT>(std::forward<Args>(args)...));
    return *this;
  }
  template <class CommandT>
    requires std::is_base_of_v<Command, CommandT>
  CommandInvoker &addCommand(CommandT &&cmd) {
    m_commands.push_back(
      std::make_unique<CommandT>(std::forward<CommandT>(cmd)));
    return *this;
  }

  CommandInvoker &addCommand(std::unique_ptr<Command> &&cmd) {
    m_commands.push_back(std::move(cmd));
    return *this;
  }

  template <typename... Args> auto executeAll(Args &&...args) {
    auto count = 0;
    do {
      auto commands = std::exchange(m_commands, {});
      for (auto &cmd : commands) {
        if (!cmd->execute(std::forward<Args>(args)...)) continue;

        if (cmd->canUndo()) {
          m_history.resize(m_historyIndex + 1);
          m_history.push_back(std::move(cmd));
          m_historyIndex = static_cast<int32_t>(m_history.size()) - 1;
        }
        ++count;
      }
    } while (!m_commands.empty());
    return count;
  }

  bool canUndo() const { return m_historyIndex >= 0; }
  bool canRedo() const {
    return m_historyIndex < static_cast<int32_t>(m_history.size()) - 1;
  }
  template <typename... Args> CommandInvoker &undo(Args &&...args) {
    if (canUndo()) {
      m_history[m_historyIndex]->undo(std::forward<Args>(args)...);
      m_historyIndex--;
    }
    return *this;
  }
  template <typename... Args> CommandInvoker &redo(Args &&...args) {
    if (canRedo()) {
      m_historyIndex++;
      m_history[m_historyIndex]->execute(std::forward<Args>(args)...);
    }
    return *this;
  }

  [[nodiscard]] auto getHistoryIndex() const { return m_historyIndex; }

  CommandInvoker &clear() {
    m_commands.clear();
    m_history.clear();
    m_historyIndex = -1;
    return *this;
  }

private:
  using CommandList = std::vector<std::unique_ptr<Command>>;
  CommandList m_commands;
  CommandList m_history;
  int32_t m_historyIndex{-1};
};
