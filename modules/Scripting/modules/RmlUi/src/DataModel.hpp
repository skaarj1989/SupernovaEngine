#pragma once

#include "sol/table.hpp"
#include "RmlUi/Core/DataModelHandle.h"

class ScriptDataModel {
public:
  ScriptDataModel(Rml::Context &, const Rml::String &, sol::table &);

  [[nodiscard]] Rml::DataModelHandle getModelHandle() const;

private:
  Rml::DataModelConstructor m_constructor;
};

void registerDataModel(sol::table &);
