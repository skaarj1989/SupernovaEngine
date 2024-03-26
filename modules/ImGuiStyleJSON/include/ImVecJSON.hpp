#pragma once

#include "nlohmann/json_fwd.hpp"

struct ImVec2;
struct ImVec4;

void from_json(const nlohmann::json &j, ImVec2 &out);
void to_json(nlohmann::ordered_json &j, const ImVec2 &in);

void from_json(const nlohmann::json &j, ImVec4 &out);
void to_json(nlohmann::ordered_json &j, const ImVec4 &in);
