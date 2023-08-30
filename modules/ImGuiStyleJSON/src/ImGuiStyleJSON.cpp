#include "ImGuiStyleJSON.hpp"
#include "ImVecJSON.hpp"
#include <fstream>

#define MAKE_PAIR(Prefix, Value)                                               \
  { Prefix##_##Value, #Value }

#define MAKE_COLOR_PAIR(Value) MAKE_PAIR(ImGuiCol, Value)
NLOHMANN_JSON_SERIALIZE_ENUM(ImGuiCol_,
                             {
                               MAKE_COLOR_PAIR(Text),
                               MAKE_COLOR_PAIR(TextDisabled),
                               MAKE_COLOR_PAIR(WindowBg),
                               MAKE_COLOR_PAIR(ChildBg),
                               MAKE_COLOR_PAIR(PopupBg),
                               MAKE_COLOR_PAIR(Border),
                               MAKE_COLOR_PAIR(BorderShadow),
                               MAKE_COLOR_PAIR(FrameBg),
                               MAKE_COLOR_PAIR(FrameBgHovered),
                               MAKE_COLOR_PAIR(FrameBgActive),
                               MAKE_COLOR_PAIR(TitleBg),
                               MAKE_COLOR_PAIR(TitleBgActive),
                               MAKE_COLOR_PAIR(TitleBgCollapsed),
                               MAKE_COLOR_PAIR(MenuBarBg),
                               MAKE_COLOR_PAIR(ScrollbarBg),
                               MAKE_COLOR_PAIR(ScrollbarGrab),
                               MAKE_COLOR_PAIR(ScrollbarGrabHovered),
                               MAKE_COLOR_PAIR(ScrollbarGrabActive),
                               MAKE_COLOR_PAIR(CheckMark),
                               MAKE_COLOR_PAIR(SliderGrab),
                               MAKE_COLOR_PAIR(SliderGrabActive),
                               MAKE_COLOR_PAIR(Button),
                               MAKE_COLOR_PAIR(ButtonHovered),
                               MAKE_COLOR_PAIR(ButtonActive),
                               MAKE_COLOR_PAIR(Header),
                               MAKE_COLOR_PAIR(HeaderHovered),
                               MAKE_COLOR_PAIR(HeaderActive),
                               MAKE_COLOR_PAIR(Separator),
                               MAKE_COLOR_PAIR(SeparatorHovered),
                               MAKE_COLOR_PAIR(SeparatorActive),
                               MAKE_COLOR_PAIR(ResizeGrip),
                               MAKE_COLOR_PAIR(ResizeGripHovered),
                               MAKE_COLOR_PAIR(ResizeGripActive),
                               MAKE_COLOR_PAIR(Tab),
                               MAKE_COLOR_PAIR(TabHovered),
                               MAKE_COLOR_PAIR(TabActive),
                               MAKE_COLOR_PAIR(TabUnfocused),
                               MAKE_COLOR_PAIR(TabUnfocusedActive),
                               MAKE_COLOR_PAIR(DockingEmptyBg),
                               MAKE_COLOR_PAIR(PlotLines),
                               MAKE_COLOR_PAIR(PlotLinesHovered),
                               MAKE_COLOR_PAIR(PlotHistogram),
                               MAKE_COLOR_PAIR(PlotHistogramHovered),
                               MAKE_COLOR_PAIR(TableHeaderBg),
                               MAKE_COLOR_PAIR(TableBorderStrong),
                               MAKE_COLOR_PAIR(TableBorderLight),
                               MAKE_COLOR_PAIR(TableRowBg),
                               MAKE_COLOR_PAIR(TableRowBgAlt),
                               MAKE_COLOR_PAIR(TextSelectedBg),
                               MAKE_COLOR_PAIR(DragDropTarget),
                               MAKE_COLOR_PAIR(NavHighlight),
                               MAKE_COLOR_PAIR(NavWindowingHighlight),
                               MAKE_COLOR_PAIR(NavWindowingDimBg),
                               MAKE_COLOR_PAIR(ModalWindowDimBg),
                             });
#undef MAKE_COLOR_PAIR
static_assert(ImGuiCol_COUNT == 55);

#define MAKE_DIR_PAIR(Value) MAKE_PAIR(ImGuiDir, Value)
NLOHMANN_JSON_SERIALIZE_ENUM(ImGuiDir_, {
                                          MAKE_DIR_PAIR(None),
                                          MAKE_DIR_PAIR(Left),
                                          MAKE_DIR_PAIR(Right),
                                          MAKE_DIR_PAIR(Up),
                                          MAKE_DIR_PAIR(Down),
                                        });
#undef MAKE_DIR_PAIR
static_assert(ImGuiDir_COUNT == 4);

#undef MAKE_PAIR

using ColorMap = std::map<ImGuiCol_, ImVec4>;

static void from_json(const nlohmann::json &j, ImGuiStyle &out) {
#define GET_VALUE(Name, ...) out.Name = j.value(#Name, __VA_ARGS__)

  // Default values taken from ImGuiStyle::ImGuiStyle()

  GET_VALUE(DisabledAlpha, 0.60f);
  GET_VALUE(WindowPadding, ImVec2{8, 8});
  GET_VALUE(WindowRounding, 0.0f);
  GET_VALUE(WindowBorderSize, 1.0f);
  GET_VALUE(WindowMinSize, ImVec2{32, 32});
  GET_VALUE(WindowTitleAlign, ImVec2{0.0f, 0.5f});
  GET_VALUE(WindowMenuButtonPosition, ImGuiDir_Left);
  GET_VALUE(ChildRounding, 0.0f);
  GET_VALUE(ChildBorderSize, 1.0f);
  GET_VALUE(PopupRounding, 0.0f);
  GET_VALUE(PopupBorderSize, 1.0f);
  GET_VALUE(FramePadding, ImVec2{4, 3});
  GET_VALUE(FrameRounding, 0.0f);
  GET_VALUE(FrameBorderSize, 0.0f);
  GET_VALUE(ItemSpacing, ImVec2{8, 4});
  GET_VALUE(ItemInnerSpacing, ImVec2{4, 4});
  GET_VALUE(CellPadding, ImVec2{4, 2});
  GET_VALUE(TouchExtraPadding, ImVec2{0, 0});
  GET_VALUE(IndentSpacing, 21.0f);
  GET_VALUE(ColumnsMinSpacing, 6.0f);
  GET_VALUE(ScrollbarSize, 14.0f);
  GET_VALUE(ScrollbarRounding, 9.0f);
  GET_VALUE(GrabMinSize, 12.0f);
  GET_VALUE(GrabRounding, 0.0f);
  GET_VALUE(LogSliderDeadzone, 4.0f);
  GET_VALUE(TabRounding, 4.0f);
  GET_VALUE(TabBorderSize, 0.0f);
  GET_VALUE(TabMinWidthForCloseButton, 0.0f);
  GET_VALUE(ColorButtonPosition, ImGuiDir_Right);
  GET_VALUE(ButtonTextAlign, ImVec2{0.5f, 0.5f});
  GET_VALUE(SelectableTextAlign, ImVec2{0.0f, 0.0f});
  GET_VALUE(SeparatorTextBorderSize, 3.0f);
  GET_VALUE(SeparatorTextAlign, ImVec2{0.0f, 0.5f});
  GET_VALUE(SeparatorTextPadding, ImVec2{20.0f, 3.0f});
  GET_VALUE(DisplayWindowPadding, ImVec2{19, 19});
  GET_VALUE(DisplaySafeAreaPadding, ImVec2{3, 3});
  GET_VALUE(MouseCursorScale, 1.0f);
  GET_VALUE(AntiAliasedLines, true);
  GET_VALUE(AntiAliasedLinesUseTex, true);
  GET_VALUE(AntiAliasedFill, true);
  GET_VALUE(CurveTessellationTol, 1.25f);
  GET_VALUE(CircleTessellationMaxError, 1.25f);

#undef GET_VALUE

  if (j.contains("Colors")) {
    for (const auto &[key, value] : j["Colors"].get<ColorMap>())
      out.Colors[key] = value;
  }
}
static void to_json(nlohmann::ordered_json &j, const ImGuiStyle &in) {
  constexpr auto convert = [](const ImVec4 in[]) {
    ColorMap out;
    for (auto i = 0; i < ImGuiCol_COUNT; ++i) {
      out.emplace(ImGuiCol_(i), in[i]);
    }
    return out;
  };

#define STORE_VALUE(Name) {#Name, in.Name}
#define STORE_VALUE_EX(Name, ConvertTo)                                        \
  { #Name, ConvertTo(in.Name) }

  j = nlohmann::ordered_json{
    STORE_VALUE(Alpha),
    STORE_VALUE(DisabledAlpha),
    STORE_VALUE(WindowPadding),
    STORE_VALUE(WindowRounding),
    STORE_VALUE(WindowBorderSize),
    STORE_VALUE(WindowMinSize),
    STORE_VALUE(WindowTitleAlign),
    STORE_VALUE_EX(WindowMenuButtonPosition, ImGuiDir_),
    STORE_VALUE(ChildRounding),
    STORE_VALUE(ChildBorderSize),
    STORE_VALUE(PopupRounding),
    STORE_VALUE(PopupBorderSize),
    STORE_VALUE(FramePadding),
    STORE_VALUE(FrameRounding),
    STORE_VALUE(FrameBorderSize),
    STORE_VALUE(ItemSpacing),
    STORE_VALUE(ItemInnerSpacing),
    STORE_VALUE(CellPadding),
    STORE_VALUE(TouchExtraPadding),
    STORE_VALUE(IndentSpacing),
    STORE_VALUE(ColumnsMinSpacing),
    STORE_VALUE(ScrollbarSize),
    STORE_VALUE(ScrollbarRounding),
    STORE_VALUE(GrabMinSize),
    STORE_VALUE(GrabRounding),
    STORE_VALUE(LogSliderDeadzone),
    STORE_VALUE(TabRounding),
    STORE_VALUE(TabMinWidthForCloseButton),
    STORE_VALUE_EX(ColorButtonPosition, ImGuiDir_),
    STORE_VALUE(ButtonTextAlign),
    STORE_VALUE(SelectableTextAlign),
    STORE_VALUE(SeparatorTextBorderSize),
    STORE_VALUE(SeparatorTextAlign),
    STORE_VALUE(SeparatorTextPadding),
    STORE_VALUE(DisplaySafeAreaPadding),
    STORE_VALUE(MouseCursorScale),
    STORE_VALUE(AntiAliasedLines),
    STORE_VALUE(AntiAliasedLinesUseTex),
    STORE_VALUE(AntiAliasedFill),
    STORE_VALUE(CurveTessellationTol),
    STORE_VALUE(CircleTessellationMaxError),
    {"Colors", convert(in.Colors)},
  };
}

bool save(const std::filesystem::path &p, const ImGuiStyle &style) {
  std::ofstream f{p};
  if (!f.is_open()) return false;

  f << std::setw(2) << nlohmann::ordered_json{style}.front() << std::endl;
  return true;
}
bool load(const std::filesystem::path &p, ImGuiStyle &style) {
  std::ifstream f{p};
  if (!f.is_open()) return false;

  ImGuiStyle temp{style};
  try {
    temp = nlohmann::json::parse(f);
  } catch (const nlohmann::json::exception &) {
    return false;
  }
  style = std::move(temp);
  return true;
}
