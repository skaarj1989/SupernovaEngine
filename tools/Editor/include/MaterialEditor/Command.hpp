#pragma once

#include "spdlog/fwd.h"

class Command {
public:
  virtual ~Command() = default;

  struct Context {
    spdlog::logger &logger;
  };
  virtual bool execute(const Context &) = 0;
  virtual bool undo(const Context &) { return false; }

  virtual bool canUndo() const { return false; }
};

class UndoableCommand : public Command {
public:
  bool canUndo() const final { return true; }
};
