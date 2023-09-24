#pragma once

#include "RmlUi/Core/SystemInterface.h"
#include "RmlUi/Core/FileInterface.h"
#include <chrono>

class RmlUiSystemInterface : public Rml::SystemInterface {
public:
  RmlUiSystemInterface();

  double GetElapsedTime() override;

private:
  using clock = std::chrono::high_resolution_clock;
  clock::time_point m_start;
};

class RmlUiFileInterface : public Rml::FileInterface {
public:
  Rml::FileHandle Open(const Rml::String &path) override;
  void Close(Rml::FileHandle) override;

  size_t Read(void *buffer, size_t size, Rml::FileHandle) override;
  bool Seek(Rml::FileHandle, long offset, int origin) override;
  size_t Tell(Rml::FileHandle) override;
};
