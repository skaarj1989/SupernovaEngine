#include "ImVecJSON.hpp"
#include "nlohmann/json.hpp"
#include "imgui.h"

void from_json(const nlohmann::json &j, ImVec2 &out) { out = {j[0], j[1]}; }
void to_json(nlohmann::ordered_json &j, const ImVec2 &in) { j = {in.x, in.y}; }

void from_json(const nlohmann::json &j, ImVec4 &out) {
  out = {j[0], j[1], j[2], j[3]};
}
void to_json(nlohmann::ordered_json &j, const ImVec4 &in) {
  j = {in.x, in.y, in.z, in.w};
}
