#pragma once

#include "sol/forward.hpp"
#include "ScriptedFunctionData.hpp"
#include <expected>

void registerMaterialNodes(sol::state &);

std::expected<std::pair<uint32_t, ScriptedFunctionData>, std::string>
loadFunction(const std::filesystem::path &, sol::state &);
