#pragma once

#include "sol/forward.hpp"
#include "MaterialEditor/ScriptedFunction.hpp"
#include <expected>

void registerMaterialNodes(sol::state &);

std::expected<ScriptedFunction, std::string>
loadFunction(const std::filesystem::path &, sol::state &);
