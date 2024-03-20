#include "ImNodesStyleJSON.hpp"
#include "ImVecJSON.hpp"
#include "imnodes.h"
#include <fstream>

#define MAKE_COLOR_PAIR(Value)                                                 \
  { ImNodesCol##_##Value, #Value }
// clang-format off
NLOHMANN_JSON_SERIALIZE_ENUM(ImNodesCol_, {
  MAKE_COLOR_PAIR(NodeBackground),
  MAKE_COLOR_PAIR(NodeBackgroundHovered),
  MAKE_COLOR_PAIR(NodeBackgroundSelected),
  MAKE_COLOR_PAIR(NodeOutline),
  MAKE_COLOR_PAIR(TitleBar),
  MAKE_COLOR_PAIR(TitleBarHovered),
  MAKE_COLOR_PAIR(TitleBarSelected),
  MAKE_COLOR_PAIR(Link),
  MAKE_COLOR_PAIR(LinkHovered),
  MAKE_COLOR_PAIR(LinkSelected),
  MAKE_COLOR_PAIR(Pin),
  MAKE_COLOR_PAIR(PinHovered),
  MAKE_COLOR_PAIR(BoxSelector),
  MAKE_COLOR_PAIR(BoxSelectorOutline),
  MAKE_COLOR_PAIR(GridBackground),
  MAKE_COLOR_PAIR(GridLine),
  MAKE_COLOR_PAIR(GridLinePrimary),
  MAKE_COLOR_PAIR(MiniMapBackground),
  MAKE_COLOR_PAIR(MiniMapBackgroundHovered),
  MAKE_COLOR_PAIR(MiniMapOutline),
  MAKE_COLOR_PAIR(MiniMapOutlineHovered),
  MAKE_COLOR_PAIR(MiniMapNodeBackground),
  MAKE_COLOR_PAIR(MiniMapNodeBackgroundHovered),
  MAKE_COLOR_PAIR(MiniMapNodeBackgroundSelected),
  MAKE_COLOR_PAIR(MiniMapNodeOutline),
  MAKE_COLOR_PAIR(MiniMapLink),
  MAKE_COLOR_PAIR(MiniMapLinkSelected),
  MAKE_COLOR_PAIR(MiniMapCanvas),
  MAKE_COLOR_PAIR(MiniMapCanvasOutline),
});
// clang-format on
#undef MAKE_COLOR_PAIR
static_assert(ImNodesCol_COUNT == 29);

using ColorMap = std::map<ImNodesCol_, ImVec4>;

static void from_json(const nlohmann::json &j, ImNodesStyle &out) {
#define GET_VALUE(Name, ...) out.Name = j.value(#Name, __VA_ARGS__)

  GET_VALUE(GridSpacing, 24.0f);
  GET_VALUE(NodeCornerRounding, 4.0f);
  GET_VALUE(NodePadding, ImVec2{8.0f, 8.0f});
  GET_VALUE(NodeBorderThickness, 1.0f);
  GET_VALUE(LinkThickness, 3.0f);
  GET_VALUE(LinkLineSegmentsPerLength, 0.1f);
  GET_VALUE(LinkHoverDistance, 10.0f);
  GET_VALUE(PinCircleRadius, 4.0f);
  GET_VALUE(PinQuadSideLength, 7.0f);
  GET_VALUE(PinTriangleSideLength, 9.5f);
  GET_VALUE(PinLineThickness, 1.0f);
  GET_VALUE(PinHoverRadius, 10.0f);
  GET_VALUE(PinOffset, 0.0f);
  GET_VALUE(MiniMapPadding, ImVec2{8.0f, 8.0f});
  GET_VALUE(MiniMapOffset, ImVec2{4.0f, 4.0f});
  GET_VALUE(Flags, ImNodesStyleFlags_NodeOutline | ImNodesStyleFlags_GridLines);

#undef GET_VALUE

  if (j.contains("Colors")) {
    for (const auto &[key, value] : j["Colors"].get<ColorMap>()) {
      out.Colors[key] = ImColor{value};
    }
  }
}
static void to_json(nlohmann::ordered_json &j, const ImNodesStyle &in) {
  const auto convert = [](const uint32_t in[]) {
    ColorMap out;
    for (auto i = 0; i < ImNodesCol_COUNT; ++i) {
      out.emplace(
        std::pair{ImNodesCol_(i), ImGui::ColorConvertU32ToFloat4(in[i])});
    }
    return out;
  };

#define STORE_VALUE(Name) {#Name, in.Name}

  // clang-format off
  j = nlohmann::ordered_json{
    STORE_VALUE(GridSpacing),
    STORE_VALUE(NodeCornerRounding),
    STORE_VALUE(NodePadding),
    STORE_VALUE(NodeBorderThickness),
    STORE_VALUE(LinkThickness),
    STORE_VALUE(LinkLineSegmentsPerLength),
    STORE_VALUE(LinkHoverDistance),
    STORE_VALUE(PinCircleRadius),
    STORE_VALUE(PinQuadSideLength),
    STORE_VALUE(PinTriangleSideLength),
    STORE_VALUE(PinLineThickness),
    STORE_VALUE(PinHoverRadius),
    STORE_VALUE(PinOffset),
    STORE_VALUE(MiniMapPadding),
    STORE_VALUE(MiniMapOffset),
    STORE_VALUE(Flags),
    {"Colors", convert(in.Colors)},
  };
  // clang-format on

#undef STORE_VALUE
}

bool save(const std::filesystem::path &p, const ImNodesStyle &style) {
  if (std::ofstream f{p, std::ios::trunc}; f.is_open()) {
    f << std::setw(2) << nlohmann::ordered_json{style}[0] << std::endl;
    return true;
  }
  return false;
}
bool load(const std::filesystem::path &p, ImNodesStyle &style) {
  std::ifstream f{p};
  ImNodesStyle temp{style};
  try {
    temp = nlohmann::json::parse(f);
  } catch (const std::exception &) {
    return false;
  }
  style = std::move(temp);
  return true;
}
