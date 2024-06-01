// Redesigned https://github.com/CedricGuillemet/ImGuizmo

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#  define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ImGuizmo.hpp"
#include "imgui_internal.h" // ImRect, ImQsort

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/common.hpp"        // fmod
#include "glm/gtx/compatibility.hpp" // lerp
#include "glm/gtx/transform.hpp"     // translate, rotate, scale

#pragma warning(disable : 26812) // Silence unscoped enum

//-----------------------------------------------------------------------------
// [SECTION] CONSTANTS
//-----------------------------------------------------------------------------

constexpr auto kPi = glm::pi<float>();
constexpr auto kEpsilon = glm::epsilon<float>();

static const glm::vec3 kUnitDirections[3]{
  {1.0f, 0.0f, 0.0f}, // Right
  {0.0f, 1.0f, 0.0f}, // Up
  {0.0f, 0.0f, 1.0f}  // Forward
};

// The size of the quads responsible for movement on a plane.
constexpr float kQuadSize{0.20f};
constexpr float kQuadMin{0.30f};
constexpr float kQuadMax{kQuadMin + kQuadSize};
static const float kUnitQuad[]{kQuadMin, kQuadMin, kQuadMin, kQuadMax,
                               kQuadMax, kQuadMax, kQuadMax, kQuadMin};

constexpr float kCircleRadius{6.0f};  // Translation and scale dots.
constexpr float kLineThickness{3.0f}; // Translation and scale axes.
constexpr int32_t kCircleSegmentCount{128};

constexpr float kOuterAnchorSize{6.0f};
constexpr float kMidAnchorSize{4.0f};

//-----------------------------------------------------------------------------
// [SECTION] INTERNAL TYPES
//-----------------------------------------------------------------------------

enum ImGuizmoAxis_ {
  ImGuizmoAxis_X,
  ImGuizmoAxis_Y,
  ImGuizmoAxis_Z,

  ImGuizmoAxis_COUNT
};
using ImGuizmoAxis = int32_t;

// Plane index is also an index to axis that is normal to that plane.
enum ImGuizmoPlane_ {
  ImGuizmoPlane_YZ,
  ImGuizmoPlane_ZX,
  ImGuizmoPlane_XY,

  ImGuizmoPlane_COUNT
};
using ImGuizmoPlane = int32_t;

struct ImGuizmoRay {
  glm::vec3 Origin{0.0f};
#ifdef _DEBUG
  glm::vec3 End{0.0f};
#endif
  glm::vec3 Direction{0.0f};
};
struct ImGuizmoCamera {
  bool IsOrtho{false};

  glm::mat4 ViewMatrix{1.0f};
  glm::mat4 ProjectionMatrix{1.0f};
  glm::mat4 ViewProjectionMatrix{1.0f};

  glm::vec3 Right{kUnitDirections[0]};
  glm::vec3 Up{kUnitDirections[1]};
  glm::vec3 Forward{kUnitDirections[2]};
  glm::vec3 Eye{0.0f};
};

struct ImGuizmoWidget {
  ImGuiID ID{0};

  bool Dirty{false}; // It's set to true on manipulate.
  bool Visible{false};
  bool Hovered{false};

  // Used as a reference model matrix, doesn't change while manipulating.
  glm::mat4 SourceModelMatrix{1.0f};
  glm::mat4 ModelMatrix{1.0f};
  glm::mat4 InversedModelMatrix{1.0f};

  glm::mat4 DeltaMatrix{1.0f};

  glm::mat4 ModelViewProjMatrix{1.0f};

  ImGuizmoMode Mode{ImGuizmoMode_Local};
  ImGuizmoOperation ActiveOperation{ImGuizmoOperation_None};
  ImGuizmoAxisFlags ActiveManipulationFlags{ImGuizmoAxisFlags_None};
  ImGuizmoAxisFlags LockedAxesFlags{ImGuizmoAxisFlags_None};

  // Screen space values:

  glm::vec2 Origin{0.0f};
  float RingRadius{0.0f};
  float ScreenFactor{0.0f};

  // Shared across transformations:

  glm::vec4 TranslationPlane{0.0f};       // T+R+S
  glm::vec3 TranslationPlaneOrigin{0.0f}; // T+S
  glm::vec3 ModelRelativeOrigin{0.0f};    // T+S
  glm::vec3 DragTranslationOrigin{0.0f};  // T+S

  // Translation:

  glm::vec3 LastTranslationDelta{0.0f};

  // Rotation:

  glm::vec3 ModelScaleOrigin{1.0f};
  glm::vec3 RotationVectorSource{0.0f};
  float RotationAngle{0.0f};       // In radians.
  float RotationAngleOrigin{0.0f}; // In radians.

  // Scale:

  glm::vec3 Scale{1.0f};
  glm::vec3 LastScale{1.0f};
  glm::vec3 ScaleValueOrigin{1.0f};

  // ---

  explicit ImGuizmoWidget(ImGuiID id) : ID{id} {}
  void Load(const glm::mat4 &model);
  float CalculateAngleOnPlane() const;
};

struct ImGuizmoBounds {
  glm::vec3 OuterPoints[3][4]{};
  glm::vec3 MidPoints[3][4]{};

  glm::vec3 Anchor{0.0f};
  glm::vec3 LocalPivot{0.0f};
  glm::vec3 Pivot{0.0f};

  ImGuizmoPlane ActivePlane{-1};
  int32_t ActiveBoundIdx{-1};
};

//-----------------------------------------------------------------------------
// [SECTION] CONTEXT
//-----------------------------------------------------------------------------

struct ImGuizmoContext {
  ImDrawList *DrawList{nullptr};

  bool Enabled{true};

  ImGuizmoStyle Style;
  ImGuizmoConfigFlags ConfigFlags{ImGuizmoConfigFlags_HasReversing};

  ImRect Viewport;
  ImGuizmoCamera Camera;
  ImGuizmoRay Ray;
  glm::vec2 DragOrigin{0.0f};

  ImVector<ImGuizmoWidget *> Gizmos;
  ImGuiStorage GizmosById;

  ImGuizmoWidget *CurrentGizmo{nullptr}; // Gizmo in Begin/End scope.
  ImGuizmoWidget *ActiveGizmo{nullptr};  // Currently manipulated gizmo.

  ImGuizmoBounds Bounds;

  // Local planes, 0 = invisible, 1 = most visible.
  float PlanesVisibility[3]{0.0f};
  ImGuizmoPlane MostVisiblePlanes[3]{0, 1, 2}; // Local planes.

  glm::mat4 *LockedModelMatrix{nullptr};
  glm::mat4 BackupModelMatrix{1.0f}; // For undoing operation.

  // ---

  ImGuizmoContext() = default;
  ~ImGuizmoContext();

  float GetAspectRatio() const;
};
static ImGuizmoContext GImGuizmo;

static ImGuizmoWidget *FindGizmoById(ImGuiID id) {
  return static_cast<ImGuizmoWidget *>(GImGuizmo.GizmosById.GetVoidPtr(id));
}
static ImGuizmoWidget *CreateNewGizmo(ImGuiID id) {
  ImGuizmoContext &g{GImGuizmo};
  auto *gizmo = new ImGuizmoWidget(id);
  g.GizmosById.SetVoidPtr(id, gizmo);
  g.Gizmos.push_back(gizmo);
  return gizmo;
}
static ImGuizmoWidget *GetCurrentGizmo() { return GImGuizmo.CurrentGizmo; }

ImGuizmoStyle::ImGuizmoStyle() { ImGuizmo::StyleColorsClassic(this); }

namespace ImGuizmo {

//-----------------------------------------------------------------------------
// [SECTION] STYLING
//-----------------------------------------------------------------------------

static void CorrectGamma(ImVec4 *colors) {
  constexpr auto kGamma = 2.2f;
  for (auto i = 0; i < ImGuizmoCol_COUNT; ++i) {
    const glm::vec4 &src_color{colors[i]};
    colors[i] = glm::vec4{
      glm::pow(glm::vec3{src_color}, glm::vec3{1.0f / kGamma}), src_color.a};
  }
}

ImGuizmoStyle &GetStyle() { return GImGuizmo.Style; }
void StyleColorsClassic(ImGuizmoStyle *dst) {
  ImGuizmoStyle *style{dst ? dst : &ImGuizmo::GetStyle()};
  ImVec4 *colors{style->Colors};

  colors[ImGuizmoCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
  colors[ImGuizmoCol_TextShadow] = ImVec4{0.0f, 0.0f, 0.0f, 1.0f};

  colors[ImGuizmoCol_Inactive] = ImVec4{0.6f, 0.6f, 0.6f, 0.6f};
  colors[ImGuizmoCol_Hovered] = ImVec4{1.0f, 0.5f, 0.06f, 0.54f};

  colors[ImGuizmoCol_SpecialMove] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};

  colors[ImGuizmoCol_AxisX] = ImVec4{0.66f, 0.0f, 0.0f, 1.0f};
  colors[ImGuizmoCol_AxisY] = ImVec4{0.0f, 0.66f, 0.0f, 1.0f};
  colors[ImGuizmoCol_AxisZ] = ImVec4{0.0f, 0.0f, 0.66f, 1.0f};

  colors[ImGuizmoCol_PlaneYZ] = ImVec4{0.66f, 0.0f, 0.0f, 0.38f};
  colors[ImGuizmoCol_PlaneZX] = ImVec4{0.0f, 0.66f, 0.0f, 0.38f};
  colors[ImGuizmoCol_PlaneXY] = ImVec4{0.0f, 0.0f, 0.66f, 0.38f};

  colors[ImGuizmoCol_BoundAnchor] = ImVec4{0.66f, 0.66f, 0.66f, 1.0f};
}
void StyleColorsBlender(ImGuizmoStyle *dst) {
  ImGuizmoStyle *style{dst ? dst : &ImGuizmo::GetStyle()};
  ImVec4 *colors{style->Colors};

  colors[ImGuizmoCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
  colors[ImGuizmoCol_TextShadow] = ImVec4{0.0f, 0.0f, 0.0f, 1.0f};

  colors[ImGuizmoCol_Inactive] = ImVec4{0.6f, 0.6f, 0.6f, 0.6f};
  colors[ImGuizmoCol_Hovered] = ImVec4{1.0f, 0.5f, 0.06f, 1.0f};

  colors[ImGuizmoCol_SpecialMove] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};

  colors[ImGuizmoCol_AxisX] = ImVec4{1.0f, 0.2f, 0.321f, 1.0f};
  colors[ImGuizmoCol_AxisY] = ImVec4{0.545f, 0.862f, 0.0f, 1.0f};
  colors[ImGuizmoCol_AxisZ] = ImVec4{0.156f, 0.564f, 1.0f, 1.0f};

  colors[ImGuizmoCol_PlaneYZ] = ImVec4{1.0f, 0.2f, 0.321f, 0.6f};
  colors[ImGuizmoCol_PlaneZX] = ImVec4{0.545f, 0.862f, 0.0f, 0.6f};
  colors[ImGuizmoCol_PlaneXY] = ImVec4{0.156f, 0.564f, 1.0f, 0.6f};

  colors[ImGuizmoCol_BoundAnchor] = ImVec4{0.66f, 0.66f, 0.66f, 1.0f};
}
void StyleColorsUnreal(ImGuizmoStyle *dst) {
  ImGuizmoStyle *style{dst ? dst : &ImGuizmo::GetStyle()};
  ImVec4 *colors{style->Colors};

  colors[ImGuizmoCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
  colors[ImGuizmoCol_TextShadow] = ImVec4{0.0f, 0.0f, 0.0f, 1.0f};

  colors[ImGuizmoCol_Inactive] = ImVec4{0.7f, 0.7f, 0.7f, 0.7f};
  colors[ImGuizmoCol_Hovered] = ImVec4{1.0f, 1.0f, 0.0f, 1.0f};

  colors[ImGuizmoCol_SpecialMove] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};

  colors[ImGuizmoCol_AxisX] = ImVec4{0.594f, 0.0197f, 0.0f, 1.0f};
  colors[ImGuizmoCol_AxisY] = ImVec4{0.1349f, 0.3959f, 0.0f, 1.0f};
  colors[ImGuizmoCol_AxisZ] = ImVec4{0.0251f, 0.207f, 0.85f, 1.0f};

  colors[ImGuizmoCol_PlaneYZ] = ImVec4{0.594f, 0.0197f, 0.0f, 0.6f};
  colors[ImGuizmoCol_PlaneZX] = ImVec4{0.1349f, 0.3959f, 0.0f, 0.6f};
  colors[ImGuizmoCol_PlaneXY] = ImVec4{0.0251f, 0.207f, 0.85f, 0.6f};

  colors[ImGuizmoCol_BoundAnchor] = ImVec4{0.66f, 0.66f, 0.66f, 1.0f};

  CorrectGamma(colors);
}

static const char *GetStyleColorName(ImGuizmoCol idx) {
#define CASE(Value)                                                            \
  case ImGuizmoCol_##Value:                                                    \
    return #Value

  switch (idx) {
    CASE(Text);
    CASE(TextShadow);
    CASE(Inactive);
    CASE(Hovered);
    CASE(SpecialMove);
    CASE(AxisX);
    CASE(AxisY);
    CASE(AxisZ);
    CASE(PlaneYZ);
    CASE(PlaneZX);
    CASE(PlaneXY);
    CASE(BoundAnchor);
  }
#undef CASE

  IM_ASSERT(0);
  return "Unknown";
}
void ShowStyleEditor(ImGuizmoStyle *ref) {
  ImGuizmoStyle &style{ImGuizmo::GetStyle()};
  static ImGuizmoStyle ref_saved_style{};

  static auto init = true;
  if (init && ref == nullptr) ref_saved_style = style;
  init = false;
  if (ref == nullptr) ref = &ref_saved_style;

  // TODO ...

  ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

  if (ImGuizmo::ShowStyleSelector("Colors##Selector")) ref_saved_style = style;

  ImGui::DragFloat("Alpha", &style.Alpha, 0.01f, 0.01f, 1.0f);
  ImGui::DragFloat("GizmoScale", &style.GizmoScale, 0.01f, 0.01f, 1.0f);
  ImGui::DragFloat("RingThickness", &style.RotationRingThickness, 0.1f, 0.1f,
                   10.0f);

  static ImGuiColorEditFlags alpha_flags{ImGuiColorEditFlags_AlphaPreviewHalf};
  for (auto i = 0; i < ImGuizmoCol_COUNT; ++i) {
    const char *name{ImGuizmo::GetStyleColorName(i)};

    ImGui::PushID(i);
    ImGui::ColorEdit4("##color", (float *)&style.Colors[i],
                      ImGuiColorEditFlags_AlphaBar | alpha_flags);
    ImGui::SameLine();
    ImGui::TextUnformatted(name);
    ImGui::PopID();
  }
}
bool ShowStyleSelector(const char *label) {
  static int32_t style_idx{-1};
  if (ImGui::Combo(label, &style_idx, "Classic\0Blender\0Unreal\0\0")) {
    switch (style_idx) {
    case 0:
      StyleColorsClassic();
      break;
    case 1:
      StyleColorsBlender();
      break;
    case 2:
      StyleColorsUnreal();
      break;
    }
    return true;
  }
  return false;
}

static ImU32 GetColorU32(ImGuizmoCol idx, float alpha_mul = 1.0f) {
  const auto &style = GImGuizmo.Style;
  auto c = style.Colors[idx];
  c.w *= style.Alpha * alpha_mul;
  return ImGui::ColorConvertFloat4ToU32(c);
}
static const ImVec4 &GetStyleColorVec4(ImGuizmoCol idx) {
  return GImGuizmo.Style.Colors[idx];
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (Geometry functions)
//-----------------------------------------------------------------------------

static glm::vec2 WorldToScreen(const glm::vec3 &world_pos,
                               const glm::mat4 &view_proj_matrix,
                               const ImRect &viewport) {
  auto temp = view_proj_matrix * glm::vec4{world_pos, 1.0f};
  temp *= 0.5f / temp.w;

  auto clip_pos = glm::vec2{temp} + 0.5f;
  clip_pos.y = 1.0f - clip_pos.y;
  return clip_pos * viewport.GetSize() + viewport.GetTL();
}
static glm::vec2 WorldToScreen(const glm::vec3 &world_pos,
                               const glm::mat4 &view_proj_matrix) {
  return WorldToScreen(world_pos, view_proj_matrix, GImGuizmo.Viewport);
}

static glm::vec2 ScreenToNDC(const ImVec2 &p, const ImRect &viewport) {
  return {
    (2.0f * p.x) / viewport.GetWidth() - 1.0f,
    1.0f - (2.0f * p.y) / viewport.GetHeight(),
  };
}
static glm::vec3 Unproject(const glm::vec3 &p,
                           const glm::mat4 &inversed_view_proj) {
  auto temp = inversed_view_proj * glm::vec4{p, 1.0f};
  return glm::vec3{temp} / temp.w;
}
static ImGuizmoRay RayCast(const glm::mat4 &view_proj_matrix,
                           const ImRect &viewport) {
  // https://antongerdelan.net/opengl/raycasting.html
  // https://github.com/opengl-tutorials/ogl/blob/master/misc05_picking/misc05_picking_custom.cpp

  const auto inversed_view_proj = glm::inverse(view_proj_matrix);
  const auto p =
    ScreenToNDC(ImGui::GetIO().MousePos - viewport.GetTL(), viewport);

  const auto ray_origin_world_space =
    Unproject(glm::vec3{p, 0.0f}, inversed_view_proj);
  const auto ray_end_world_space =
    Unproject(glm::vec3{p, 1.0f}, inversed_view_proj);

  return ImGuizmoRay{
    ray_origin_world_space,
#ifdef _DEBUG
    ray_end_world_space,
#endif
    glm::normalize(ray_end_world_space - ray_origin_world_space)};
}

static glm::vec4 BuildPlane(const glm::vec3 &point, const glm::vec3 &normal) {
  const auto n = glm::normalize(normal);
  return glm::vec4{n, glm::dot(n, point)};
}
static float DistanceToPlane(const glm::vec3 &point, const glm::vec4 &plane) {
  return glm::dot(glm::vec3{plane}, point) + plane.w;
}
static float IntersectRayPlane(const ImGuizmoRay &ray, const glm::vec4 &plane) {
  const auto num = glm::dot(glm::vec3{plane}, ray.Origin) - plane.w;
  const auto denom = glm::dot(glm::vec3{plane}, ray.Direction);
  // Normal is orthogonal to vector, can't intersect.
  return glm::abs(denom) < kEpsilon ? -1.0f : -(num / denom);
}

static glm::vec2 PointOnSegment(const glm::vec2 &point, const glm::vec2 &v1,
                                const glm::vec2 &v2) {
  const auto c = point - v1;
  const auto V = glm::normalize(v2 - v1);
  const auto t = glm::dot(V, c);
  if (t < 0.0f) return v1;
  const auto d = glm::length(v2 - v1);
  if (t > d) return v2;

  return v1 + V * t;
}
static float GetSegmentLengthClipSpace(const glm::vec3 &start,
                                       const glm::vec3 &end) {
  const ImGuizmoContext &g{GImGuizmo};
  const ImGuizmoWidget *gizmo{GetCurrentGizmo()};

  auto start_of_segment = gizmo->ModelViewProjMatrix * glm::vec4{start, 1.0f};
  // Check for axis aligned with camera direction.
  if (glm::abs(start_of_segment.w) > kEpsilon)
    start_of_segment *= 1.0f / start_of_segment.w;

  auto end_of_segment = gizmo->ModelViewProjMatrix * glm::vec4{end, 1.0f};
  // Check for axis aligned with camera direction.
  if (glm::abs(end_of_segment.w) > kEpsilon)
    end_of_segment *= 1.0f / end_of_segment.w;

  glm::vec2 clip_space_axis{end_of_segment - start_of_segment};
  clip_space_axis.y /= g.GetAspectRatio();
  return glm::length(clip_space_axis);
}

//-----------------------------------------------------------------------------
// [SECTION] UTILITIES (SNAP)
//-----------------------------------------------------------------------------

static void CalculateSnap(float &value, float snap) {
  if (snap <= kEpsilon) return;

  const auto modulo = glm::fmod(value, snap);
  const auto modulo_ratio = glm::abs(modulo) / snap;
  constexpr auto kSnapTension = 0.5f;
  if (modulo_ratio < kSnapTension) {
    value -= modulo;
  } else if (modulo_ratio > (1.0f - kSnapTension)) {
    value = value - modulo + snap * ((value < 0.0f) ? -1.0f : 1.0f);
  }
}
static void CalculateSnap(glm::vec3 &value, const float snap) {
  for (auto axis_idx = 0; axis_idx < 3; ++axis_idx) {
    CalculateSnap(value[axis_idx], snap);
  }
}

//-----------------------------------------------------------------------------
// [SECTION] UTILITIES
//-----------------------------------------------------------------------------

static void Print(const char *label, const glm::vec3 &v) {
  ImGui::Text("%s = [%.2f, %.2f, %.2f]", label, v.x, v.y, v.z);
}
static void PrintPlane(const char *label, const glm::vec4 &plane) {
  ImGui::Text("%s = {\n\tnormal = [%.2f, %.2f, %.2f]\n\tdistance = %.2f\n}",
              label, plane.x, plane.y, plane.z, plane.w);
}

static const char *StatOperation(ImGuizmoOperation operation) {
#define CASE(Value)                                                            \
  case ImGuizmoOperation_##Value:                                              \
    return #Value

  switch (operation) {
    CASE(None);
    CASE(Translate);
    CASE(Rotate);
    CASE(Scale);
    CASE(BoundsScale);
  }
#undef CASE

  IM_ASSERT(0);
  return "Unknown";
}

static const char *StatAxisFlags(ImGuizmoAxisFlags flags) {
#define CASE(Value)                                                            \
  case ImGuizmoAxisFlags_##Value:                                              \
    return #Value

  switch (flags) {
    CASE(None);
    CASE(X);
    CASE(Y);
    CASE(Z);
    CASE(YZ);
    CASE(ZX);
    CASE(XY);
    CASE(ALL);
  }
#undef CASE

  IM_ASSERT(0);
  return "Unknown";
}

static const char *GetAxisName(ImGuizmoAxis axis_idx) {
#define CASE(Value)                                                            \
  case ImGuizmoAxis_##Value:                                                   \
    return #Value

  switch (axis_idx) {
    CASE(X);
    CASE(Y);
    CASE(Z);
  }
#undef CASE

  IM_ASSERT(0);
  return "Unknown";
}
static bool HasSingleAxis(ImGuizmoAxisFlags flags) {
  return flags == ImGuizmoAxisFlags_X || flags == ImGuizmoAxisFlags_Y ||
         flags == ImGuizmoAxisFlags_Z;
}
static ImGuizmoAxis GetAxisIdx(ImGuizmoAxisFlags flags, bool around) {
  switch (flags) {
  case ImGuizmoAxisFlags_X:
    return around ? ImGuizmoAxis_Z : ImGuizmoAxis_X;
  case ImGuizmoAxisFlags_Y:
    return ImGuizmoAxis_Y;
  case ImGuizmoAxisFlags_Z:
    return around ? ImGuizmoAxis_X : ImGuizmoAxis_Z;
  }
  IM_ASSERT(0);
  return -1;
}
static ImGuizmoAxis GetAxisAroundIdx(ImGuizmoAxis axis_idx) {
  switch (axis_idx) {
  case ImGuizmoAxis_X:
    return ImGuizmoAxis_Z;
  case ImGuizmoAxis_Y:
    return ImGuizmoAxis_Y;
  case ImGuizmoAxis_Z:
    return ImGuizmoAxis_X;
  }
  IM_ASSERT(0);
  return -1;
}
static ImGuizmoAxisFlags AxisToFlag(ImGuizmoAxis axis_idx, bool around) {
  switch (axis_idx) {
  case ImGuizmoAxis_X:
    return around ? ImGuizmoAxisFlags_Z : ImGuizmoAxisFlags_X;
  case ImGuizmoAxis_Y:
    return ImGuizmoAxisFlags_Y;
  case ImGuizmoAxis_Z:
    return around ? ImGuizmoAxisFlags_X : ImGuizmoAxisFlags_Z;
  }
  IM_ASSERT(0);
  return -1;
}

static const char *GetPlaneName(ImGuizmoPlane plane_idx) {
#define CASE(Value)                                                            \
  case ImGuizmoPlane_##Value:                                                  \
    return #Value

  switch (plane_idx) {
    CASE(YZ);
    CASE(ZX);
    CASE(XY);
  }
#undef CASE

  IM_ASSERT(0);
  return "Unknown";
}
static bool HasPlane(ImGuizmoAxisFlags flags) {
  if (flags == ImGuizmoAxisFlags_ALL) return false;
  return flags == ImGuizmoAxisFlags_YZ || flags == ImGuizmoAxisFlags_ZX ||
         flags == ImGuizmoAxisFlags_XY;
}
static ImGuizmoPlane GetPlaneIdx(ImGuizmoAxisFlags flags) {
#define CASE(Value)                                                            \
  case ImGuizmoAxisFlags_##Value:                                              \
    return ImGuizmoPlane_##Value

  switch (flags) {
    CASE(YZ);
    CASE(ZX);
    CASE(XY);
  }
#undef CASE

  IM_ASSERT(0);
  return -1;
}
static ImGuizmoAxisFlags PlaneToFlags(ImGuizmoPlane plane_idx) {
#define CASE(Value)                                                            \
  case ImGuizmoPlane_##Value:                                                  \
    return ImGuizmoAxisFlags_##Value

  switch (plane_idx) {
    CASE(YZ);
    CASE(ZX);
    CASE(XY);
  }
#undef CASE

  IM_ASSERT(0);
  return -1;
}

//-----------------------------------------------------------------------------
// [SECTION]
//-----------------------------------------------------------------------------

static bool GizmoBehavior(ImGuizmoOperation operation,
                          ImGuizmoAxisFlags &hover_flags, bool *out_held) {
  IM_ASSERT(operation);

  ImGuizmoContext &g{GImGuizmo};
  auto *gizmo = GetCurrentGizmo();

  if (g.ActiveGizmo != gizmo && g.ActiveGizmo) {
    hover_flags = ImGuizmoAxisFlags_None;
  } else {
    if (gizmo->ActiveOperation != operation && gizmo->ActiveOperation) {
      hover_flags = ImGuizmoAxisFlags_None;
    } else {
      if (gizmo->ActiveManipulationFlags)
        hover_flags = gizmo->ActiveManipulationFlags;
    }
  }
  bool pressed{hover_flags && ImGui::IsMouseClicked(ImGuiMouseButton_Left)};
  if (pressed) {
    g.ActiveGizmo = gizmo;
    gizmo->ActiveOperation = operation;
    gizmo->ActiveManipulationFlags = hover_flags;
  }

  bool held{false};
  if (gizmo->ActiveManipulationFlags == hover_flags &&
      gizmo->ActiveManipulationFlags) {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      held = true;
    } else {
      g.ActiveGizmo = nullptr;
      gizmo->ActiveManipulationFlags = ImGuizmoAxisFlags_None;
    }
  }
  if (out_held) *out_held = held;

  if (gizmo->ActiveManipulationFlags != ImGuizmoAxisFlags_None)
    ImGui::SetNextFrameWantCaptureMouse(true);

  gizmo->Hovered = hover_flags != ImGuizmoAxisFlags_None;

  return pressed;
}
static bool BoundBehavior(ImGuizmoAxisFlags &hover_flags,
                          ImGuizmoPlane &hovered_plane_idx,
                          int32_t &hovered_bound_idx, bool *out_held) {
  ImGuizmoContext &g{GImGuizmo};

  bool held;
  const auto pressed =
    GizmoBehavior(ImGuizmoOperation_BoundsScale, hover_flags, &held);

  if (pressed) {
    g.Bounds.ActivePlane = hovered_plane_idx;
    g.Bounds.ActiveBoundIdx = hovered_bound_idx;
  }
  if (held) {
    hovered_plane_idx = g.Bounds.ActivePlane;
    hovered_bound_idx = g.Bounds.ActiveBoundIdx;
  }
  if (out_held) *out_held = held;
  return pressed;
}

static void Begin(const ImGuizmoWidget &gizmo) {
  ImGuizmoContext &g{GImGuizmo};
  g.BackupModelMatrix = gizmo.SourceModelMatrix;
  g.DragOrigin = ImGui::GetIO().MousePos;
}

//-----------------------------------------------------------------------------
// [SECTION] HOVER QUERY
//-----------------------------------------------------------------------------

static bool CanActivate() {
  return ImGui::IsWindowHovered() &&
         GImGuizmo.Viewport.Contains(ImGui::GetIO().MousePos);
}

static bool IsCoreHovered() {
  const ImGuiIO &io{ImGui::GetIO()};
  const auto *gizmo = GetCurrentGizmo();

  if (gizmo->LockedAxesFlags == ImGuizmoAxisFlags_ALL) return false;

  constexpr auto kMargin = 3.0f;
  const auto distance = glm::length(glm::vec2{io.MousePos} - gizmo->Origin);
  return distance <= kCircleRadius + kMargin;
}

static bool IsAxisHovered(ImGuizmoAxis axis_idx) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  if (gizmo->LockedAxesFlags & AxisToFlag(axis_idx, false)) return false;

  const auto dir_axis =
    glm::vec3{gizmo->ModelMatrix * glm::vec4{kUnitDirections[axis_idx], 0.0f}};
  const auto length =
    IntersectRayPlane(g.Ray, BuildPlane(gizmo->ModelMatrix[3], dir_axis));
  const auto mouse_pos_on_plane = WorldToScreen(
    g.Ray.Origin + g.Ray.Direction * length, g.Camera.ViewProjectionMatrix);

  constexpr auto kAxisShift = 0.1f;
  const auto axis_start_on_screen =
    WorldToScreen(glm::vec3{gizmo->ModelMatrix[3]} +
                    dir_axis * gizmo->ScreenFactor * kAxisShift,
                  g.Camera.ViewProjectionMatrix);
  const auto axis_end_on_screen = WorldToScreen(
    glm::vec3{gizmo->ModelMatrix[3]} + dir_axis * gizmo->ScreenFactor,
    g.Camera.ViewProjectionMatrix);
  const auto closest_point_on_axis = PointOnSegment(
    mouse_pos_on_plane, axis_start_on_screen, axis_end_on_screen);
  constexpr auto kTolerance = 6.0f;
  return glm::length(closest_point_on_axis - mouse_pos_on_plane) < kTolerance;
}
static bool IsPlaneHovered(ImGuizmoPlane plane_idx) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  if (gizmo->LockedAxesFlags & PlaneToFlags(plane_idx)) return false;

  const auto plane_normal =
    glm::vec3{gizmo->ModelMatrix * glm::vec4{kUnitDirections[plane_idx], 0.0f}};
  const auto length =
    IntersectRayPlane(g.Ray, BuildPlane(gizmo->ModelMatrix[3], plane_normal));
  const auto mouse_pos_on_plane = g.Ray.Origin + g.Ray.Direction * length;

  const auto plane_dir1 = glm::vec3{
    gizmo->ModelMatrix * glm::vec4{kUnitDirections[(plane_idx + 1) % 3], 0.0f}};
  const auto dx = glm::dot(
    plane_dir1, (mouse_pos_on_plane - glm::vec3{gizmo->ModelMatrix[3]}) *
                  (1.0f / gizmo->ScreenFactor));
  const auto plane_dir2 = glm::vec3{
    gizmo->ModelMatrix * glm::vec4{kUnitDirections[(plane_idx + 2) % 3], 0.0f}};
  const auto dy = glm::dot(
    plane_dir2, (mouse_pos_on_plane - glm::vec3{gizmo->ModelMatrix[3]}) *
                  (1.0f / gizmo->ScreenFactor));
  return (dx >= kUnitQuad[0] && dx <= kUnitQuad[4] && dy >= kUnitQuad[1] &&
          dy <= kUnitQuad[3]);
}

static bool IsRotationAxisHovered(ImGuizmoAxis axis_idx) {
  const ImGuiIO &io{ImGui::GetIO()};
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  if (gizmo->LockedAxesFlags == AxisToFlag(axis_idx, false)) return false;

  const auto pickup_plane =
    BuildPlane(gizmo->ModelMatrix[3], gizmo->ModelMatrix[axis_idx]);
  const auto length = IntersectRayPlane(g.Ray, pickup_plane);
  const auto local_pos = glm::normalize(
    g.Ray.Origin + g.Ray.Direction * length - glm::vec3{gizmo->ModelMatrix[3]});

  if (glm::dot(local_pos, g.Ray.Direction) > kEpsilon) return false;

  const auto ideal_pos_on_circle =
    glm::vec3{gizmo->InversedModelMatrix * glm::vec4{local_pos, 0.0f}};
  const auto ideal_pos_on_circle_screen_space = WorldToScreen(
    ideal_pos_on_circle * gizmo->ScreenFactor, gizmo->ModelViewProjMatrix);

  constexpr auto kTolerance = 8.0f;
  const auto distance_on_screen =
    ideal_pos_on_circle_screen_space - glm::vec2{io.MousePos};
  return glm::length(distance_on_screen) < kTolerance;
}
static bool IsRotationRingHovered() {
  const ImGuiIO &io{ImGui::GetIO()};
  const ImGuizmoStyle &style{GetStyle()};
  const auto *gizmo = GetCurrentGizmo();

  if (gizmo->LockedAxesFlags == ImGuizmoAxisFlags_ALL) return false;

  constexpr auto kTolerance = 1.0f;
  const auto ring_thickness = style.RotationRingThickness + kTolerance;
  const auto distance = glm::length(glm::vec2{io.MousePos} - gizmo->Origin);
  return (distance >= gizmo->RingRadius - ring_thickness) &&
         (distance < gizmo->RingRadius + ring_thickness);
}

//-----------------------------------------------------------------------------
// [SECTION]
//-----------------------------------------------------------------------------

static float CalculatePlaneVisibility(ImGuizmoPlane plane_idx, bool local) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  const auto &matrix = local ? gizmo->SourceModelMatrix : gizmo->ModelMatrix;
  const glm::vec3 plane_normal{
    glm::normalize(matrix * glm::vec4{kUnitDirections[plane_idx], 0.0f})};
  return glm::abs(glm::dot(glm::normalize(g.Camera.Eye - glm::vec3{matrix[3]}),
                           plane_normal));
}
static bool IsAxisVisible(ImGuizmoAxis axis_idx) {
  const auto *gizmo = GetCurrentGizmo();

  constexpr auto kVisibilityThreshold = 0.03f;
  const auto axis_length = GetSegmentLengthClipSpace(
    glm::vec3{0.0f}, kUnitDirections[axis_idx] * gizmo->ScreenFactor);
  return axis_length >= kVisibilityThreshold;
}
static bool IsPlaneVisible(ImGuizmoPlane plane_idx, bool local = true) {
  const ImGuizmoContext &g{GImGuizmo};

  assert(plane_idx < 3);
  constexpr auto kThreshold = 0.2f;
  const auto visibility = local ? g.PlanesVisibility[plane_idx]
                                : CalculatePlaneVisibility(plane_idx, false);
  return visibility >= kThreshold;
}

//-----------------------------------------------------------------------------
// [SECTION] RENDERING
//-----------------------------------------------------------------------------

// COLOR:

static ImU32 GetSpecialMoveColor(ImGuizmoAxisFlags hover_flags) {
  const auto *gizmo = GetCurrentGizmo();

  auto color = GetColorU32(ImGuizmoCol_SpecialMove);
  if (gizmo->LockedAxesFlags == ImGuizmoAxisFlags_ALL)
    color = GetColorU32(ImGuizmoCol_Inactive);
  else if (hover_flags == ImGuizmoAxisFlags_ALL)
    color = GetColorU32(ImGuizmoCol_Hovered, 0.541f);

  return color;
}
static ImU32 GetAxisColor(ImGuizmoAxis axis_idx, ImGuizmoAxisFlags hover_flags,
                          bool around) {
  const auto *gizmo = GetCurrentGizmo();

  auto color = GetColorU32(ImGuizmoCol_AxisX +
                           (around ? GetAxisAroundIdx(axis_idx) : axis_idx));
  if (gizmo->LockedAxesFlags & AxisToFlag(axis_idx, around)) {
    color = GetColorU32(ImGuizmoCol_Inactive);
  } else if (HasSingleAxis(hover_flags) &&
             GetAxisIdx(hover_flags, around) == axis_idx) {
    color = GetColorU32(ImGuizmoCol_Hovered, 0.541f);
  }
  return color;
}
static ImU32 GetPlaneColor(ImGuizmoPlane plane_idx,
                           ImGuizmoAxisFlags hover_flags) {
  const auto *gizmo = GetCurrentGizmo();

  auto color = GetColorU32(ImGuizmoCol_PlaneYZ + plane_idx);
  if (gizmo->LockedAxesFlags & PlaneToFlags(plane_idx))
    color = GetColorU32(ImGuizmoCol_Inactive);
  else if (HasPlane(hover_flags) && GetPlaneIdx(hover_flags) == plane_idx)
    color = GetColorU32(ImGuizmoCol_Hovered, 0.541f);

  return color;
}
static ImU32 GetBoundColor(bool hovered) {
  return hovered ? GetColorU32(ImGuizmoCol_Hovered, 0.541f)
                 : GetColorU32(ImGuizmoCol_BoundAnchor);
}

// TRANSLATION:

static void RenderCore(ImGuizmoAxisFlags hover_flags) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  if (g.ConfigFlags & ImGuizmoConfigFlags_HideLocked &&
      gizmo->LockedAxesFlags == ImGuizmoAxisFlags_ALL) {
    return;
  }

  const auto color = GetSpecialMoveColor(hover_flags);
  g.DrawList->AddCircleFilled(gizmo->Origin, kCircleRadius, color,
                              kCircleSegmentCount);
}

static void RenderArrowhead(const glm::vec2 &head_pos, ImU32 color) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  constexpr auto kArrowheadSize = kLineThickness * 2.0f;
  const auto dir = glm::normalize(gizmo->Origin - head_pos) * kArrowheadSize;
  const glm::vec2 orthogonal_dir{dir.y, -dir.x};
  const glm::vec2 a{head_pos + dir};
  g.DrawList->AddTriangleFilled(head_pos - dir, a + orthogonal_dir,
                                a - orthogonal_dir, color);
}
static void RenderTranslateAxis(ImGuizmoAxis axis_idx,
                                ImGuizmoAxisFlags hover_flags) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  if (!IsAxisVisible(axis_idx)) return;
  if (g.ConfigFlags & ImGuizmoConfigFlags_HideLocked &&
      gizmo->LockedAxesFlags & AxisToFlag(axis_idx, false)) {
    return;
  }

  const auto &dir_axis = kUnitDirections[axis_idx];
  const auto tail_pos = WorldToScreen(dir_axis * 0.1f * gizmo->ScreenFactor,
                                      gizmo->ModelViewProjMatrix);

  const auto head_pos =
    WorldToScreen(dir_axis * gizmo->ScreenFactor, gizmo->ModelViewProjMatrix);

  const auto color = GetAxisColor(axis_idx, hover_flags, false);
  g.DrawList->AddLine(tail_pos, head_pos, color, kLineThickness);
  RenderArrowhead(head_pos, color);
}

static ImU32 AdjustAlpha(ImU32 color, float alpha) {
  auto v = ImGui::ColorConvertU32ToFloat4(color);
  v.w = glm::clamp(alpha, alpha, v.w);
  return ImGui::ColorConvertFloat4ToU32(v);
}

static void RenderPlane(ImGuizmoPlane plane_idx,
                        ImGuizmoAxisFlags hover_flags) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  if (!IsPlaneVisible(plane_idx, gizmo->Mode == ImGuizmoMode_Local)) return;

  if (g.ConfigFlags & ImGuizmoConfigFlags_HideLocked &&
      gizmo->LockedAxesFlags & PlaneToFlags(plane_idx)) {
    return;
  }

  ImVec2 plane_points[4];
  for (auto i = 0; i < 4; ++i) {
    const glm::vec3 corner_world_space{
      (kUnitDirections[(plane_idx + 1) % 3] * kUnitQuad[i * 2] +
       kUnitDirections[(plane_idx + 2) % 3] * kUnitQuad[i * 2 + 1]) *
      gizmo->ScreenFactor};
    plane_points[i] =
      WorldToScreen(corner_world_space, gizmo->ModelViewProjMatrix);
  }

  // const auto color = GetPlaneColor(plane_idx, hover_flags);
  const auto color = AdjustAlpha(GetPlaneColor(plane_idx, hover_flags),
                                 g.PlanesVisibility[plane_idx]);
  g.DrawList->AddConvexPolyFilled(plane_points, 4, color);
  constexpr auto kPlaneBorder = 1.5f;
  g.DrawList->AddPolyline(plane_points, 4, color | 0x20'00'00'00, true,
                          kPlaneBorder);
}
static void RenderTranslationTrail() {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  const auto tail_pos =
    WorldToScreen(gizmo->DragTranslationOrigin, g.Camera.ViewProjectionMatrix);
  const auto head_pos =
    WorldToScreen(gizmo->ModelMatrix[3], g.Camera.ViewProjectionMatrix);
  const auto diff =
    glm::normalize(head_pos - tail_pos) * (kCircleRadius - 1.0f);

  constexpr ImU32 kTrailLineColor{0xAA'AA'AA'AA};
  constexpr auto kMargin = 1.5f;
  g.DrawList->AddCircle(tail_pos, kCircleRadius + kMargin, kTrailLineColor);
  g.DrawList->AddCircle(head_pos, kCircleRadius + kMargin, kTrailLineColor);
  g.DrawList->AddLine(tail_pos + diff, head_pos - diff, kTrailLineColor,
                      kLineThickness / 2);
}

// ROTATION:

static void RenderRotationAxis(ImGuizmoAxis axis_idx, bool circle,
                               ImGuizmoAxisFlags hover_flags) {
  const ImGuizmoContext &g{GImGuizmo};
  auto *gizmo = GetCurrentGizmo();

  if (g.ConfigFlags & ImGuizmoConfigFlags_HideLocked &&
      gizmo->LockedAxesFlags & AxisToFlag(axis_idx, true)) {
    return;
  }

  glm::vec3 camera_to_model_normalized =
    g.Camera.IsOrtho
      ? -glm::inverse(g.Camera.ViewMatrix)[2]
      : glm::normalize(glm::vec3{gizmo->ModelMatrix[3]} - g.Camera.Eye);
  camera_to_model_normalized =
    gizmo->InversedModelMatrix * glm::vec4{camera_to_model_normalized, 0.0f};

  const auto angle_start =
    (glm::atan(camera_to_model_normalized[(4 - axis_idx) % 3],
               camera_to_model_normalized[(3 - axis_idx) % 3])) +
    kPi * 0.5f;
  ImVec2 circle_pos[kCircleSegmentCount];
  for (auto i = 0; i < kCircleSegmentCount; ++i) {
    const auto ng =
      angle_start +
      (circle ? 2 : 1) * kPi * (static_cast<float>(i) / kCircleSegmentCount);
    const glm::vec3 axis_pos{glm::cos(ng), glm::sin(ng), 0.0f};
    const auto pos =
      glm::vec3{
        axis_pos[axis_idx],
        axis_pos[(axis_idx + 1) % 3],
        axis_pos[(axis_idx + 2) % 3],
      } *
      gizmo->ScreenFactor;
    circle_pos[i] = WorldToScreen(pos, gizmo->ModelViewProjMatrix);
  }

  const auto color = GetAxisColor(axis_idx, hover_flags, true);
  g.DrawList->AddPolyline(circle_pos, kCircleSegmentCount, color, circle,
                          kLineThickness);
}
static void RenderRotationRing(ImGuizmoAxisFlags hover_flags) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  if (g.ConfigFlags & ImGuizmoConfigFlags_HideLocked &&
      gizmo->LockedAxesFlags == ImGuizmoAxisFlags_ALL) {
    return;
  }

  const auto color = GetSpecialMoveColor(hover_flags);
  g.DrawList->AddCircle(gizmo->Origin, gizmo->RingRadius, color,
                        kCircleSegmentCount, g.Style.RotationRingThickness);
}
static void RenderRotationTrail() {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  ImU32 border_color;
  ImU32 color;
#if 1
  border_color = GetColorU32(ImGuizmoCol_Hovered);
  color = GetColorU32(ImGuizmoCol_Hovered, 0.541f);
#else
  const ImGuizmoAxisFlags hover_flags{gizmo->ActiveManipulationFlags};
  if (HasSingleAxis(hover_flags)) {
    const ImGuizmoAxis axis_idx{GetAxisIdx(hover_flags, false)};
    color = GetColorU32(ImGuizmoCol_AxisX + axis_idx, 0.541f);
    border_color = GetColorU32(ImGuizmoCol_AxisX + axis_idx);
  } else {
    color = GetColorU32(ImGuizmoCol_SpecialMove, 0.541f);
    border_color = GetColorU32(ImGuizmoCol_SpecialMove);
  }
#endif

  ImVec2 circle_points[kCircleSegmentCount + 1]{gizmo->Origin};
  for (auto i = 1; i < kCircleSegmentCount; ++i) {
    const auto ng = gizmo->RotationAngle *
                    (static_cast<float>(i - 1) / (kCircleSegmentCount - 1));
    const auto rotate_vector_matrix =
      glm::rotate(ng, glm::vec3{gizmo->TranslationPlane});
    glm::vec3 pos{rotate_vector_matrix *
                  glm::vec4{gizmo->RotationVectorSource, 1.0f}};
    pos *= gizmo->ScreenFactor;
    circle_points[i] = WorldToScreen(pos + glm::vec3{gizmo->ModelMatrix[3]},
                                     g.Camera.ViewProjectionMatrix);
  }

  g.DrawList->AddConvexPolyFilled(circle_points, kCircleSegmentCount, color);
  g.DrawList->AddPolyline(circle_points, kCircleSegmentCount, border_color,
                          true, kLineThickness);
}

// SCALE:

static void RenderScaleAxis(ImGuizmoAxis axis_idx,
                            ImGuizmoAxisFlags hover_flags) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  if (!IsAxisVisible(axis_idx)) return;
  if (g.ConfigFlags & ImGuizmoConfigFlags_HideLocked &&
      gizmo->LockedAxesFlags & AxisToFlag(axis_idx, false)) {
    return;
  }

  const auto &dir_axis = kUnitDirections[axis_idx];
  const auto tail_pos = WorldToScreen(dir_axis * 0.1f * gizmo->ScreenFactor,
                                      gizmo->ModelViewProjMatrix);
  const auto head_pos =
    WorldToScreen(dir_axis * gizmo->ScreenFactor, gizmo->ModelViewProjMatrix);

  const auto color = GetAxisColor(axis_idx, hover_flags, false);
  g.DrawList->AddLine(tail_pos, head_pos, color, kLineThickness);
#if 1
  g.DrawList->AddCircleFilled(head_pos, kCircleRadius, color);
#else
  const glm::vec2 head_scaled{
    WorldToScreen((dir_axis * gizmo->Scale[axis_idx]) * gizmo->ScreenFactor,
                  gizmo->ModelViewProjMatrix)};

  constexpr float kQuadSize{kLineThickness * 2.0f};
  const glm::vec2 dir{glm::normalize(gizmo->Origin - head_scaled) * kQuadSize};
  const glm::vec2 a{head_scaled + dir}, b{head_scaled - dir};
  const ImVec2 points[4]{
    a + glm::vec2{dir.y, -dir.x}, a - glm::vec2{dir.y, -dir.x},
    b + glm::vec2{-dir.y, dir.x}, b - glm::vec2{-dir.y, dir.x}};
  g.DrawList->AddConvexPolyFilled(points, 4, color);
#endif
}
static void RenderScaleTrail(ImGuizmoAxis axis_idx) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  const auto head_pos = WorldToScreen(
    kUnitDirections[axis_idx] * gizmo->Scale[axis_idx] * gizmo->ScreenFactor,
    gizmo->ModelViewProjMatrix);
  g.DrawList->AddCircleFilled(head_pos, kCircleRadius, 0xFF'FF'FF'FF);
}

// BOUNDS:

static void RenderDottedLine(const glm::vec2 &point_a, const glm::vec2 &point_b,
                             ImU32 color) {
  ImGuizmoContext &g{GImGuizmo};

  const auto distance = glm::distance(point_a, point_b);
  const auto step_count =
    glm::min(static_cast<int32_t>(distance / 15.0f), 1000);
  const auto step_length = 1.0f / float(step_count);
  for (auto i = 0; i < step_count; ++i) {
    const auto t1 = static_cast<float>(i) * step_length;
    const auto tail_pos = glm::lerp(point_a, point_b, t1);
    const auto t2 = t1 + step_length * 0.5f;
    const auto head_pos = glm::lerp(point_a, point_b, t2);
    g.DrawList->AddLine(tail_pos, head_pos, color, 1.0f);
  }
}
static void RenderAnchor(const glm::vec2 &pos, float radius, ImU32 color) {
  const ImGuizmoContext &g{GImGuizmo};

  constexpr auto kBorder = 1.2f;
  g.DrawList->AddCircleFilled(pos, radius, 0xFF'00'00'00);
  g.DrawList->AddCircleFilled(pos, radius - kBorder, color);
}
static void RenderBounds(const glm::mat4 &model_view_proj,
                         ImGuizmoAxisFlags hover_flags,
                         int32_t hovered_plane_idx, int32_t hovered_bound_idx) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  // Bounds are rendered starting from least visible plane. This is necessary in
  // case that there are common 'mid-points' between planes so we can avoid
  // drawing inactive bound over active one.
  for (auto i = 2; i >= 0; --i) {
    const auto plane_idx = g.MostVisiblePlanes[i];
    if (gizmo->ActiveOperation == ImGuizmoOperation_BoundsScale &&
        gizmo->ActiveManipulationFlags && hovered_plane_idx != plane_idx) {
      continue;
    }
    if (!IsPlaneVisible(plane_idx)) continue;

    const glm::vec3 *outer_points{g.Bounds.OuterPoints[plane_idx]};
    const glm::vec3 *mid_points{g.Bounds.MidPoints[plane_idx]};
    for (auto j = 0; j < 4; ++j) {
      const auto point = WorldToScreen(outer_points[j], model_view_proj);
      const auto next_point =
        WorldToScreen(outer_points[(j + 1) % 4], model_view_proj);
      RenderDottedLine(point, next_point, 0xAA'AA'AA'AA);

      const auto pre_match =
        plane_idx == hovered_plane_idx && j == hovered_bound_idx;
      const auto outer_bound_hovered = pre_match && HasPlane(hover_flags);
      RenderAnchor(point, kOuterAnchorSize, GetBoundColor(outer_bound_hovered));

      const auto mid_point = WorldToScreen(mid_points[j], model_view_proj);
      const auto mid_bound_hovered = pre_match && HasSingleAxis(hover_flags);
      RenderAnchor(mid_point, kMidAnchorSize, GetBoundColor(mid_bound_hovered));
    }
  }
}

// TEXT:

static void RenderText(const glm::vec2 &position, const char *text) {
  const ImGuizmoContext &g{GImGuizmo};
  g.DrawList->AddText(position + 15.0f, GetColorU32(ImGuizmoCol_TextShadow),
                      text);
  g.DrawList->AddText(position + 14.0f, GetColorU32(ImGuizmoCol_Text), text);
}

static const char *const kInfoMasks[]{
  // -- Translation:
  "X : %5.3f",                     // 0
  "Y : %5.3f",                     // 1
  "Z : %5.3f",                     // 2
  "Y : %5.3f Z : %5.3f",           // 3
  "X : %5.3f Z : %5.3f",           // 6
  "X : %5.3f Y : %5.3f",           // 9
  "X : %5.3f Y : %5.3f Z : %5.3f", // 0

  // -- Rotation:
  "X : %5.2f deg %5.2f rad",      // 0
  "Y : %5.2f deg %5.2f rad",      // 1
  "Z : %5.2f deg %5.2f rad",      // 2
  "Screen : %5.2f deg %5.2f rad", // 0

  // -- Scale:
  "XYZ : %5.2f" // 0
};
static const int32_t kInfoDataIndices[]{
  0, 1, 2, // XYZ
  1, 2, 0, // YZ (0-unused)
  0, 2, 0, // XZ (0-unused)
  0, 1, 0, // XY (0-unused)
};

static void RenderTranslationInfo() {
  const ImGuizmoWidget *gizmo{GetCurrentGizmo()};

  const auto hover_flags = gizmo->ActiveManipulationFlags;
  const char *mask{nullptr};
  int32_t start_idx{0};
  if (HasSingleAxis(hover_flags)) {
    const auto axis_idx = GetAxisIdx(hover_flags, false);
    mask = kInfoMasks[axis_idx];
    start_idx = axis_idx;
  } else if (HasPlane(hover_flags)) {
    const auto plane_idx = GetPlaneIdx(hover_flags);
    mask = kInfoMasks[ImGuizmoAxis_COUNT + plane_idx];
    start_idx = ImGuizmoAxis_COUNT + (ImGuizmoPlane_COUNT * plane_idx);
  } else {
    mask = kInfoMasks[ImGuizmoAxis_COUNT + ImGuizmoPlane_COUNT];
  }

  const auto delta_info =
    glm::vec3{gizmo->ModelMatrix[3]} - gizmo->DragTranslationOrigin;

  char info_buffer[128]{};
  ImFormatString(info_buffer, sizeof(info_buffer), mask,
                 delta_info[kInfoDataIndices[start_idx]],
                 delta_info[kInfoDataIndices[start_idx + 1]],
                 delta_info[kInfoDataIndices[start_idx + 2]]);
  RenderText(gizmo->Origin, info_buffer);
}
static void RenderRotationInfo() {
  const auto *gizmo = GetCurrentGizmo();

  const auto hover_flags = gizmo->ActiveManipulationFlags;
  const char *mask{nullptr};
  if (HasSingleAxis(hover_flags)) {
    mask = kInfoMasks[ImGuizmoAxis_COUNT + ImGuizmoPlane_COUNT + 1 +
                      GetAxisIdx(hover_flags, false)];
  } else {
    mask = kInfoMasks[(ImGuizmoAxis_COUNT * 2) + ImGuizmoPlane_COUNT];
  }

  char info_buffer[128]{};
  ImFormatString(info_buffer, sizeof(info_buffer), mask,
                 glm::degrees(gizmo->RotationAngle), gizmo->RotationAngle);
  RenderText(gizmo->Origin, info_buffer);
}
static void RenderScaleInfo(const glm::vec3 &scale) {
  const auto *gizmo = GetCurrentGizmo();

  const auto hover_flags = gizmo->ActiveManipulationFlags;
  const char *mask{nullptr};
  int32_t start_idx{0};
  if (HasSingleAxis(hover_flags)) {
    const auto axis_idx = GetAxisIdx(hover_flags, false);
    mask = kInfoMasks[axis_idx];
    start_idx = axis_idx;
  } else {
    if (glm::all(glm::equal(scale, glm::vec3{scale.x})))
      mask = kInfoMasks[11];
    else
      mask = kInfoMasks[6];
  }

  char info_buffer[128]{};
  ImFormatString(info_buffer, sizeof(info_buffer), mask,
                 scale[kInfoDataIndices[start_idx]],
                 scale[kInfoDataIndices[start_idx + 1]],
                 scale[kInfoDataIndices[start_idx + 2]]);
  RenderText(gizmo->Origin, info_buffer);
}

//-----------------------------------------------------------------------------
// [SECTION] TRANSLATION
//-----------------------------------------------------------------------------

static ImGuizmoAxisFlags FindTranslationHover() {
  if (!CanActivate()) return ImGuizmoAxisFlags_None;

  ImGuizmoAxisFlags hover_flags{ImGuizmoAxisFlags_None};
  if (IsCoreHovered()) hover_flags |= ImGuizmoAxisFlags_ALL;
  if (hover_flags != ImGuizmoAxisFlags_ALL) {
    for (auto plane_idx = 0; plane_idx < 3; ++plane_idx)
      if (IsPlaneHovered(plane_idx)) {
        hover_flags |= PlaneToFlags(plane_idx);
        break;
      }
    if (!HasPlane(hover_flags)) {
      for (auto axis_idx = 0; axis_idx < 3; ++axis_idx)
        if (IsAxisHovered(axis_idx)) {
          hover_flags |= AxisToFlag(axis_idx, false);
          break;
        }
    }
  }
  return hover_flags;
}
static glm::vec4 BuildTranslatePlane() {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  const auto hover_flags = gizmo->ActiveManipulationFlags;

  glm::vec3 move_plane_normal;
  if (HasPlane(hover_flags)) {
    move_plane_normal = gizmo->ModelMatrix[GetPlaneIdx(hover_flags)];
  } else if (HasSingleAxis(hover_flags)) {
    const glm::vec3 dir{gizmo->ModelMatrix[GetAxisIdx(hover_flags, false)]};

    const auto camera_to_model_normalized =
      glm::normalize(glm::vec3{gizmo->ModelMatrix[3]} - g.Camera.Eye);

    const auto ortho_dir = glm::cross(dir, camera_to_model_normalized);
    move_plane_normal = glm::normalize(glm::cross(dir, ortho_dir));
  } else {
    // Special movement.
    move_plane_normal = -g.Camera.Forward;
  }
  return BuildPlane(gizmo->ModelMatrix[3], move_plane_normal);
}
static void BeginTranslation() {
  ImGuizmoContext &g{GImGuizmo};
  auto *gizmo = GetCurrentGizmo();
  Begin(*gizmo);
  gizmo->DragTranslationOrigin = gizmo->ModelMatrix[3];
  gizmo->TranslationPlane = BuildTranslatePlane();
  const auto length = IntersectRayPlane(g.Ray, gizmo->TranslationPlane);
  gizmo->TranslationPlaneOrigin = g.Ray.Origin + g.Ray.Direction * length;
  gizmo->ModelRelativeOrigin =
    (gizmo->TranslationPlaneOrigin - glm::vec3{gizmo->ModelMatrix[3]}) *
    (1.0f / gizmo->ScreenFactor);
}
static void ContinueTranslation(const float *snap) {
  const ImGuizmoContext &g{GImGuizmo};
  auto *gizmo = GetCurrentGizmo();

  const auto hover_flags = gizmo->ActiveManipulationFlags;

  const auto length =
    glm::abs(IntersectRayPlane(g.Ray, gizmo->TranslationPlane));
  const auto target_position = g.Ray.Origin + g.Ray.Direction * length;
  const auto new_position =
    target_position - gizmo->ModelRelativeOrigin * gizmo->ScreenFactor;

  auto delta = new_position - glm::vec3{gizmo->ModelMatrix[3]};
  if (HasSingleAxis(hover_flags)) {
    const auto axis_idx = GetAxisIdx(hover_flags, false);
    const glm::vec3 axis_value{gizmo->ModelMatrix[axis_idx]};
    const auto length_on_axis = glm::dot(axis_value, delta);
    delta = axis_value * length_on_axis;
  }

  if (snap) {
    auto cumulative_delta =
      glm::vec3{gizmo->ModelMatrix[3]} + delta - gizmo->DragTranslationOrigin;
    const auto apply_rotation_localy =
      gizmo->Mode == ImGuizmoMode_Local || hover_flags == ImGuizmoAxisFlags_ALL;
    if (apply_rotation_localy) {
      auto source_model_normalized = gizmo->SourceModelMatrix;
      for (auto axis_idx = 0; axis_idx < 3; ++axis_idx)
        source_model_normalized[axis_idx] =
          glm::normalize(source_model_normalized[axis_idx]);
      cumulative_delta = glm::inverse(source_model_normalized) *
                         glm::vec4{cumulative_delta, 0.0f};
      CalculateSnap(cumulative_delta, *snap);
      cumulative_delta =
        source_model_normalized * glm::vec4{cumulative_delta, 0.0f};
    } else {
      CalculateSnap(cumulative_delta, *snap);
    }
    delta = gizmo->DragTranslationOrigin + cumulative_delta -
            glm::vec3{gizmo->ModelMatrix[3]};
  }

  if (delta != gizmo->LastTranslationDelta) {
    gizmo->DeltaMatrix = glm::translate(delta);
    gizmo->ModelMatrix = gizmo->DeltaMatrix * gizmo->SourceModelMatrix;
    for (auto axis_idx = 0; axis_idx < 3; ++axis_idx) {
      if (gizmo->LockedAxesFlags & AxisToFlag(axis_idx, false))
        gizmo->ModelMatrix[3][axis_idx] =
          gizmo->DragTranslationOrigin[axis_idx];
    }
    gizmo->Dirty = true;
  }
  gizmo->LastTranslationDelta = delta;
}

//-----------------------------------------------------------------------------
// [SECTION] ROTATION
//-----------------------------------------------------------------------------

static ImGuizmoAxisFlags FindRotationHover() {
  if (!CanActivate()) return ImGuizmoAxisFlags_None;

  ImGuizmoAxisFlags hover_flags{ImGuizmoAxisFlags_None};
  if (IsRotationRingHovered()) hover_flags |= ImGuizmoAxisFlags_ALL;
  if (hover_flags != ImGuizmoAxisFlags_ALL) {
    for (auto axis_idx = 0; axis_idx < 3; ++axis_idx) {
      if (IsRotationAxisHovered(axis_idx)) {
        hover_flags |= AxisToFlag(axis_idx, false);
        break;
      }
    }
  }
  return hover_flags;
}
static glm::vec4 BuildRotationPlane() {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  const auto hover_flags = gizmo->ActiveManipulationFlags;

  glm::vec3 point;
  glm::vec3 plane_normal;
  if (HasSingleAxis(hover_flags)) {
    point = gizmo->Mode == ImGuizmoMode_Local ? gizmo->ModelMatrix[3]
                                              : gizmo->SourceModelMatrix[3];
    plane_normal = gizmo->ModelMatrix[GetAxisIdx(hover_flags, false)];
  } else {
    point = gizmo->SourceModelMatrix[3];
    plane_normal = -g.Camera.Forward;
  }
  return BuildPlane(point, plane_normal);
}
static void BeginRotation() {
  const ImGuizmoContext &g{GImGuizmo};
  auto *gizmo = GetCurrentGizmo();
  Begin(*gizmo);
  gizmo->TranslationPlane = BuildRotationPlane();
  const auto length = IntersectRayPlane(g.Ray, gizmo->TranslationPlane);
  gizmo->RotationVectorSource = glm::normalize(
    g.Ray.Origin + g.Ray.Direction * length - glm::vec3{gizmo->ModelMatrix[3]});
  gizmo->RotationAngleOrigin = gizmo->CalculateAngleOnPlane();
}
static void ContinueRotation(const float *snap) {
  auto *gizmo = GetCurrentGizmo();

  gizmo->RotationAngle = gizmo->CalculateAngleOnPlane();
  if (snap) CalculateSnap(gizmo->RotationAngle, *snap);

  const glm::vec3 rotation_axis_local_space{
    glm::normalize(gizmo->InversedModelMatrix *
                   glm::vec4{glm::vec3{gizmo->TranslationPlane}, 0.0f})};

  const auto angle = gizmo->RotationAngle - gizmo->RotationAngleOrigin;
  const auto delta_rotation = glm::rotate(angle, rotation_axis_local_space);

  if (gizmo->RotationAngle != gizmo->RotationAngleOrigin) {
    // TODO Handle locked axes ...

    if (gizmo->Mode == ImGuizmoMode_Local) {
      const auto scale_origin = glm::scale(gizmo->ModelScaleOrigin);
      gizmo->DeltaMatrix = delta_rotation * scale_origin;
      gizmo->ModelMatrix *= gizmo->DeltaMatrix;
    } else {
      auto tmp = gizmo->SourceModelMatrix;
      tmp[3] = glm::vec4{glm::vec3{0.0f}, 1.0f};
      gizmo->ModelMatrix = delta_rotation * tmp;
      gizmo->ModelMatrix[3] = gizmo->SourceModelMatrix[3];

      gizmo->DeltaMatrix =
        glm::inverse(gizmo->ModelMatrix) * delta_rotation * gizmo->ModelMatrix;
    }

    gizmo->Dirty = true;
  }
  gizmo->RotationAngleOrigin = gizmo->RotationAngle;
}

//-----------------------------------------------------------------------------
// [SECTION] SCALE
//-----------------------------------------------------------------------------

static ImGuizmoAxisFlags FindScaleHover() {
  if (!CanActivate()) return ImGuizmoAxisFlags_None;

  ImGuizmoAxisFlags hover_flags{ImGuizmoAxisFlags_None};
  if (IsCoreHovered()) hover_flags |= ImGuizmoAxisFlags_ALL;
  if (hover_flags != ImGuizmoAxisFlags_ALL) {
    for (auto axis_idx = 0; axis_idx < 3; ++axis_idx)
      if (IsAxisHovered(axis_idx)) {
        hover_flags |= AxisToFlag(axis_idx, false);
        break;
      }
  }
  return hover_flags;
}
static glm::vec4 BuildScalePlane() {
  const auto *gizmo = GetCurrentGizmo();

  const auto hover_flags = gizmo->ActiveManipulationFlags;
  if (HasSingleAxis(hover_flags)) {
    const auto axis_idx = GetAxisIdx(hover_flags, false);
    return BuildPlane(gizmo->ModelMatrix[3],
                      gizmo->ModelMatrix[axis_idx == 2 ? 0 : axis_idx + 1]);
  } else {
    return BuildPlane(gizmo->ModelMatrix[3], gizmo->ModelMatrix[2]);
  }
}
static void BeginScale() {
  const ImGuizmoContext &g{GImGuizmo};
  auto *gizmo = GetCurrentGizmo();
  Begin(*gizmo);
  gizmo->Scale = glm::vec3{1.0f};
  gizmo->DragTranslationOrigin = gizmo->ModelMatrix[3];
  gizmo->TranslationPlane = BuildScalePlane();
  const auto length = IntersectRayPlane(g.Ray, gizmo->TranslationPlane);
  gizmo->TranslationPlaneOrigin = g.Ray.Origin + g.Ray.Direction * length;
  gizmo->ModelRelativeOrigin =
    (gizmo->TranslationPlaneOrigin - glm::vec3{gizmo->ModelMatrix[3]}) *
    (1.0f / gizmo->ScreenFactor);

  for (auto axis_idx = 0; axis_idx < 3; ++axis_idx)
    gizmo->ScaleValueOrigin[axis_idx] =
      glm::length(gizmo->SourceModelMatrix[axis_idx]);
}
static void ContinueScale(const float *snap) {
  const ImGuiIO &io{ImGui::GetIO()};
  const ImGuizmoContext &g{GImGuizmo};
  auto *gizmo = GetCurrentGizmo();

  const auto length = IntersectRayPlane(g.Ray, gizmo->TranslationPlane);
  const auto target_position = g.Ray.Origin + g.Ray.Direction * length;
  const auto new_position =
    target_position - gizmo->ModelRelativeOrigin * gizmo->ScreenFactor;
  auto delta = new_position - glm::vec3{gizmo->ModelMatrix[3]};

  const ImGuizmoAxisFlags hover_flags{gizmo->ActiveManipulationFlags};
  if (HasSingleAxis(hover_flags)) {
    const auto axis_idx = GetAxisIdx(hover_flags, false);
    const glm::vec3 axis_dir{gizmo->ModelMatrix[axis_idx]};
    const auto length_on_axis = glm::dot(axis_dir, delta);
    delta = axis_dir * length_on_axis;
    const auto base_vec =
      gizmo->TranslationPlaneOrigin - glm::vec3{gizmo->ModelMatrix[3]};
    const auto ratio =
      glm::dot(axis_dir, base_vec + delta) / glm::dot(axis_dir, base_vec);
    gizmo->Scale[axis_idx] = glm::max(ratio, 0.001f);
  } else {
    const auto scale_delta = (io.MousePos.x - g.DragOrigin.x) * 0.01f;
    gizmo->Scale = glm::vec3{glm::max(1.0f + scale_delta, 0.001f)};
  }

  if (snap) {
    CalculateSnap(gizmo->Scale, *snap);
  }

  for (auto axis_idx = 0; axis_idx < 3; ++axis_idx) {
    gizmo->Scale[axis_idx] = glm::max(gizmo->Scale[axis_idx], 0.001f);
    if (gizmo->LockedAxesFlags & AxisToFlag(axis_idx, false))
      gizmo->Scale[axis_idx] = 1.0f;
  }

  if (gizmo->LastScale != gizmo->Scale) {
    auto deltaScale = gizmo->Scale * gizmo->ScaleValueOrigin;
    deltaScale *= 1.0f / gizmo->ModelScaleOrigin;
    gizmo->DeltaMatrix = glm::scale(deltaScale);
    gizmo->ModelMatrix *= glm::scale(gizmo->Scale * gizmo->ScaleValueOrigin);
    gizmo->Dirty = true;
  }
  gizmo->LastScale = gizmo->Scale;
}

//-----------------------------------------------------------------------------
// [SECTION] BOUNDS SCALE
//-----------------------------------------------------------------------------

static ImGuizmoAxisFlags FindHoveredBound(const glm::mat4 &model_view_proj,
                                          ImGuizmoPlane &hovered_plane_idx,
                                          int32_t &hovered_bound_idx) {
  const ImGuiIO &io{ImGui::GetIO()};
  const ImGuizmoContext &g{GImGuizmo};

  if (!CanActivate()) return ImGuizmoAxisFlags_None;

  ImGuizmoAxisFlags hover_flags{ImGuizmoAxisFlags_None};
  for (auto i = 0; i < 3; ++i) {
    const ImGuizmoPlane plane_idx{g.MostVisiblePlanes[i]};
    if (!IsPlaneVisible(plane_idx)) continue;

    for (auto j = 0; j < 4; ++j) {
      const auto outer_bound =
        WorldToScreen(g.Bounds.OuterPoints[plane_idx][j], model_view_proj);
      const auto dir1_idx = (plane_idx + 1) % 3;
      const auto dir2_idx = (plane_idx + 2) % 3;
      if (glm::distance(outer_bound, glm::vec2{io.MousePos}) <=
          kOuterAnchorSize) {
        hovered_bound_idx = j;
        hovered_plane_idx = plane_idx;
        hover_flags = PlaneToFlags(plane_idx);
        break;
      }
      const auto mid_bound =
        WorldToScreen(g.Bounds.MidPoints[plane_idx][j], model_view_proj);
      if (glm::distance(mid_bound, glm::vec2{io.MousePos}) <= kMidAnchorSize) {
        hovered_bound_idx = j;
        hovered_plane_idx = plane_idx;
        hover_flags = AxisToFlag((j + 2) % 2 ? dir2_idx : dir1_idx, false);
        break;
      }
    }
    if (hovered_bound_idx != -1) break;
  }
  return hover_flags;
}
static void BuildOuterPoints(const float *bounds) {
  ImGuizmoContext &g{GImGuizmo};

  for (auto plane_idx = 0; plane_idx < 3; ++plane_idx) {
    const auto dir1_idx = (plane_idx + 1) % 3;
    const auto dir2_idx = (plane_idx + 2) % 3;
    const auto null_idx = (plane_idx + 3) % 3;
    for (auto i = 0; i < 4; ++i) {
      g.Bounds.OuterPoints[plane_idx][i][null_idx] = 0.0f;
      g.Bounds.OuterPoints[plane_idx][i][dir1_idx] =
        bounds[dir1_idx + 3 * (i >> 1)];
      g.Bounds.OuterPoints[plane_idx][i][dir2_idx] =
        bounds[dir2_idx + 3 * ((i >> 1) ^ (i & 1))];
    }
  }
}
static void BuildMidPoints() {
  ImGuizmoContext &g{GImGuizmo};

  for (auto plane_idx = 0; plane_idx < 3; ++plane_idx) {
    for (auto i = 0; i < 4; ++i) {
      g.Bounds.MidPoints[plane_idx][i] =
        (g.Bounds.OuterPoints[plane_idx][i] +
         g.Bounds.OuterPoints[plane_idx][(i + 1) % 4]) *
        0.5f;
    }
  }
}

static void BeginBoundsScale() {
  ImGuizmoContext &g{GImGuizmo};
  auto *gizmo = GetCurrentGizmo();
  Begin(*gizmo);

  const auto hover_flags = gizmo->ActiveManipulationFlags;
  const auto hovered_plane_idx = g.Bounds.ActivePlane;
  const auto hovered_bound_idx = g.Bounds.ActiveBoundIdx;

  const int32_t opposite_index{(hovered_bound_idx + 2) % 4};
  if (HasPlane(hover_flags)) {
    // Outer bound.
    g.Bounds.Anchor =
      gizmo->SourceModelMatrix *
      glm::vec4{g.Bounds.OuterPoints[hovered_plane_idx][hovered_bound_idx],
                1.0f};
    g.Bounds.LocalPivot =
      g.Bounds.OuterPoints[hovered_plane_idx][opposite_index];
  } else {
    // Mid bound.
    g.Bounds.Anchor =
      gizmo->SourceModelMatrix *
      glm::vec4{g.Bounds.MidPoints[hovered_plane_idx][hovered_bound_idx], 1.0f};
    g.Bounds.LocalPivot = g.Bounds.MidPoints[hovered_plane_idx][opposite_index];
  }
  g.Bounds.Pivot =
    gizmo->SourceModelMatrix * glm::vec4{g.Bounds.LocalPivot, 1.0f};

  const glm::vec3 plane_normal{
    glm::normalize(gizmo->SourceModelMatrix *
                   glm::vec4{kUnitDirections[hovered_plane_idx], 0.0f})};
  gizmo->TranslationPlane = BuildPlane(g.Bounds.Anchor, plane_normal);
}
static glm::vec3 ContinueBoundsScale(const float *bounds, const float *snap) {
  ImGuizmoContext &g{GImGuizmo};
  auto *gizmo = GetCurrentGizmo();

  const auto reference_vector = glm::abs(g.Bounds.Anchor - g.Bounds.Pivot);
  const auto length = IntersectRayPlane(g.Ray, gizmo->TranslationPlane);
  const auto target_position = g.Ray.Origin + g.Ray.Direction * length;
  const auto delta = glm::abs(target_position - g.Bounds.Pivot);

  const auto hover_flags = gizmo->ActiveManipulationFlags;
  ImGuizmoAxis axes[2]{-1, -1};
  if (HasPlane(hover_flags)) {
    axes[0] = (g.Bounds.ActivePlane + 1) % 3;
    axes[1] = (g.Bounds.ActivePlane + 2) % 3;
  } else {
    axes[0] = GetAxisIdx(hover_flags, false);
  }

  glm::mat4 scale{1.0f};
  for (const auto axis_idx : axes) {
    if (axis_idx == -1) continue;

    const glm::vec3 axis_dir{glm::abs(g.BackupModelMatrix[axis_idx])};
    const auto dt_axis = glm::dot(axis_dir, reference_vector);

    auto ratio_axis = 1.0f;
    if (dt_axis > kEpsilon) ratio_axis = glm::dot(axis_dir, delta) / dt_axis;

    if (snap) {
      const auto bound_size = bounds[axis_idx + 3] - bounds[axis_idx];
      auto length = bound_size * ratio_axis;
      CalculateSnap(length, *snap);
      if (bound_size > kEpsilon) ratio_axis = length / bound_size;
    }
    scale[axis_idx] *= ratio_axis;
  }

  gizmo->ModelMatrix = g.BackupModelMatrix *
                       glm::translate(g.Bounds.LocalPivot) * scale *
                       glm::translate(-g.Bounds.LocalPivot);
  gizmo->Dirty = true;

  glm::vec3 scale_info{};
  for (auto axis_idx = 0; axis_idx < 3; ++axis_idx) {
    scale_info[axis_idx] = (bounds[axis_idx + 3] - bounds[axis_idx]) *
                           glm::length(g.BackupModelMatrix[axis_idx]) *
                           glm::length(scale[axis_idx]);
  }
  return scale_info;
}

//-----------------------------------------------------------------------------
// [SECTION] PUBLIC INTERFACE
//-----------------------------------------------------------------------------

void PrintContext() {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = g.CurrentGizmo;

  const glm::vec2 top_left{g.Viewport.GetTL()};
  const glm::vec2 size{g.Viewport.GetSize()};
  ImGui::Text("Viewport = (%.f,%.f) %.fx%.f", top_left.x, top_left.y, size.x,
              size.y);
  ImGui::Text("DragOrigin = (%.f, %.f)", g.DragOrigin.x, g.DragOrigin.y);

  if (ImGui::TreeNode("Camera")) {
    Print("Eye", g.Camera.Eye);
    Print("Right", g.Camera.Right);
    Print("Up", g.Camera.Up);
    Print("Forward", g.Camera.Forward);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Ray")) {
    const auto &mousePos = ImGui::GetIO().MousePos;
    ImGui::Text("x: %.f, y: %.f", mousePos.x, mousePos.y);
    Print("Start", g.Ray.Origin);
#ifdef _DEBUG
    Print("End", g.Ray.End);
#endif
    Print("Direction", g.Ray.Direction);
    ImGui::TreePop();
  }

  if (gizmo) {
    ImGui::Text("ID = %d", gizmo->ID);
    ImGui::Text("ActiveOperation: %s", StatOperation(gizmo->ActiveOperation));
    ImGui::Text("ActiveManipulationFlags: %s",
                StatAxisFlags(gizmo->ActiveManipulationFlags));

    if (ImGui::TreeNode("Gizmo")) {
      if (gizmo->Hovered) ImGui::Text("[Hovered]");
      ImGui::Text("Origin: (%.2f, %.2f)", gizmo->Origin.x, gizmo->Origin.y);
      ImGui::Text("RingRadius: %.2f", gizmo->RingRadius);
      ImGui::Text("ScreenFactor: %.2f", gizmo->ScreenFactor);
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Shared")) {
      PrintPlane("TranslationPlane", gizmo->TranslationPlane);
      Print("TranslationPlaneOrigin", gizmo->TranslationPlaneOrigin);
      Print("ModelRelativeOrigin", gizmo->ModelRelativeOrigin);
      Print("DragTranslationOrigin", gizmo->DragTranslationOrigin);
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Translation")) {
      Print("LastTranslationDelta", gizmo->LastTranslationDelta);
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Rotation")) {
      Print("ModelScaleOrigin", gizmo->ModelScaleOrigin);
      Print("RotationVectorSource", gizmo->RotationVectorSource);
      ImGui::Text("RotationAngle: %.2f rad", gizmo->RotationAngle);
      ImGui::Text("RotationAngleOrigin: %.2f rad", gizmo->RotationAngleOrigin);
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Scale")) {
      Print("Scale", gizmo->Scale);
      Print("LastScale", gizmo->LastScale);
      Print("ScaleValueOrigin", gizmo->ScaleValueOrigin);
      ImGui::TreePop();
    }
  }

  if (ImGui::TreeNode("Bounds")) {
    Print("Anchor", g.Bounds.Anchor);
    Print("LocalPivot", g.Bounds.LocalPivot);
    Print("Pivot", g.Bounds.Pivot);
    ImGui::Text("ActivePlane = %s", g.Bounds.ActivePlane == -1
                                      ? "None"
                                      : GetPlaneName(g.Bounds.ActivePlane));
    ImGui::Text("ActiveBoundIdx = %d", g.Bounds.ActiveBoundIdx);
    ImGui::TreePop();
  }
}

bool IsUsing() { return GImGuizmo.ActiveGizmo; }
bool IsHovered() {
  const ImGuizmoContext &g{GImGuizmo};
  return g.CurrentGizmo && g.CurrentGizmo->Hovered;
}

void SetConfigFlags(ImGuizmoConfigFlags flags) {
  GImGuizmo.ConfigFlags = flags;
}

void SetDrawlist(ImDrawList *drawList) {
  GImGuizmo.DrawList = drawList ? drawList : ImGui::GetWindowDrawList();
}

void SetCamera(const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix,
               bool is_ortho) {
  auto &camera = GImGuizmo.Camera;

  camera.ViewMatrix = view_matrix;
  const auto inversed_view_matrix = glm::inverse(camera.ViewMatrix);
  camera.Right = inversed_view_matrix[0];
  camera.Up = inversed_view_matrix[1];
  camera.Forward = inversed_view_matrix[2];
  camera.Eye = inversed_view_matrix[3];

  camera.IsOrtho = is_ortho;
  camera.ProjectionMatrix = projection_matrix;

  camera.ViewProjectionMatrix = camera.ProjectionMatrix * camera.ViewMatrix;
}

bool Manipulate(ImGuizmoMode mode, ImGuizmoOperation operation,
                glm::mat4 &model_matrix, const float *snap) {
  if (Begin(mode, model_matrix)) {
    switch (operation) {
    case ImGuizmoOperation_Translate:
      Translate(snap ? &snap[0] : nullptr);
      break;
    case ImGuizmoOperation_Rotate:
      Rotate(snap ? &snap[1] : nullptr);
      break;
    case ImGuizmoOperation_Scale:
      Scale(snap ? &snap[2] : nullptr);
      break;
    }
  }
  return End();
}

bool Begin(ImGuizmoMode mode, glm::mat4 &model_matrix,
           ImGuizmoAxisFlags locked_axes) {
  ImGuizmoContext &g{GImGuizmo};

  IM_ASSERT(!g.LockedModelMatrix && "Nesting forbidden");
  g.LockedModelMatrix = &model_matrix;

  g.DrawList = ImGui::GetWindowDrawList();
  g.Viewport = ImGui::GetCurrentWindowRead()->InnerClipRect;

  const auto id = ImGui::GetID(&model_matrix);
  auto *gizmo = FindGizmoById(id);
  if (!gizmo) gizmo = CreateNewGizmo(id);
  g.CurrentGizmo = gizmo;

  if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    g.DragOrigin = glm::vec2{0.0f};
    g.ActiveGizmo = nullptr;
    gizmo->ActiveManipulationFlags = ImGuizmoAxisFlags_None;
  }
  if (!gizmo->ActiveManipulationFlags) {
    gizmo->ActiveOperation = ImGuizmoOperation_None;
    g.Bounds.ActivePlane = -1;
    g.Bounds.ActiveBoundIdx = -1;
  }

  gizmo->Mode = mode;
  gizmo->Load(model_matrix);
  gizmo->LockedAxesFlags = locked_axes;
  g.Ray = RayCast(g.Camera.ViewProjectionMatrix, g.Viewport);

  for (auto plane_idx = 0; plane_idx < 3; ++plane_idx)
    g.PlanesVisibility[plane_idx] = CalculatePlaneVisibility(plane_idx, true);

  ImQsort(g.MostVisiblePlanes, ImGuizmoPlane_COUNT, sizeof(ImGuizmoPlane),
          [](const void *a, const void *b) {
            const ImGuizmoContext &g{GImGuizmo};
            auto fa = g.PlanesVisibility[*static_cast<const int32_t *>(a)];
            auto fb = g.PlanesVisibility[*static_cast<const int32_t *>(b)];
            return (fa > fb) ? -1 : (fa < fb);
          });
  return gizmo->Visible;
}
bool End(glm::mat4 *delta_matrix) {
  ImGuizmoContext &g{GImGuizmo};
  IM_ASSERT(g.LockedModelMatrix && "It seems that you didn't call Begin()");

  auto *gizmo = GetCurrentGizmo();
  if (g.ConfigFlags & ImGuizmoConfigFlags_HasReversing &&
      ImGui::GetIO().MouseClicked[1] && gizmo->ActiveManipulationFlags) {
    gizmo->ModelMatrix = g.BackupModelMatrix;
    gizmo->Dirty = true;
    gizmo->ActiveManipulationFlags = ImGuizmoAxisFlags_None;
  }

  auto updated = false;
  if (gizmo->Dirty) {
    *g.LockedModelMatrix = gizmo->ModelMatrix;
    if (delta_matrix) {
      *delta_matrix = gizmo->DeltaMatrix;
    }
    gizmo->Dirty = false;
    updated = true;
  }
  g.LockedModelMatrix = nullptr;
  // g.CurrentGizmo = nullptr; // PrintContext() won't work

  return updated;
}

void Translate(const float *snap) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();
  IM_ASSERT(gizmo->Visible);

  auto hover_flags = FindTranslationHover();
  bool held;
  if (GizmoBehavior(ImGuizmoOperation_Translate, hover_flags, &held)) {
    BeginTranslation();
  }
  if (held) ContinueTranslation(snap);

  if (gizmo->ActiveOperation == ImGuizmoOperation_Translate)
    RenderTranslationTrail();

  if (!gizmo->ActiveManipulationFlags ||
      !(g.ConfigFlags & ImGuizmoConfigFlags_CloakOnManipulate)) {
    for (auto axis_idx = 0; axis_idx < 3; ++axis_idx) {
      RenderTranslateAxis(axis_idx, hover_flags);
    }
    for (auto plane_idx = 0; plane_idx < 3; ++plane_idx) {
      RenderPlane(plane_idx, hover_flags);
    }
    RenderCore(hover_flags);
  }

  if (gizmo->ActiveOperation == ImGuizmoOperation_Translate)
    RenderTranslationInfo();
}
void Rotate(const float *snap) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();
  IM_ASSERT(gizmo->Visible);

  auto hover_flags = FindRotationHover();
  bool held;
  if (GizmoBehavior(ImGuizmoOperation_Rotate, hover_flags, &held)) {
    BeginRotation();
  }
  if (held) ContinueRotation(snap);

  if (gizmo->ActiveManipulationFlags &&
      (g.ConfigFlags & ImGuizmoConfigFlags_CloakOnManipulate)) {
    if (HasSingleAxis(hover_flags)) {
      RenderRotationAxis(GetAxisIdx(hover_flags, true), true, hover_flags);
    } else if (hover_flags == ImGuizmoAxisFlags_ALL) {
      RenderRotationRing(hover_flags);
    }
  } else {
    for (auto axis_idx = 0; axis_idx < 3; ++axis_idx) {
      RenderRotationAxis(axis_idx, false, hover_flags);
    }
    RenderRotationRing(hover_flags);
  }

  if (gizmo->ActiveOperation == ImGuizmoOperation_Rotate) {
    RenderRotationTrail();
    RenderRotationInfo();
  }
}
void Scale(const float *snap) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();
  IM_ASSERT(gizmo->Visible);

  auto hover_flags = FindScaleHover();
  bool held;
  if (GizmoBehavior(ImGuizmoOperation_Scale, hover_flags, &held)) {
    BeginScale();
  }
  if (held) ContinueScale(snap);

  if (!gizmo->ActiveManipulationFlags ||
      !(g.ConfigFlags & ImGuizmoConfigFlags_CloakOnManipulate)) {
    for (auto axis_idx = 0; axis_idx < 3; ++axis_idx) {
      RenderScaleAxis(axis_idx, hover_flags);
    }
    RenderCore(hover_flags);
  }

  if (gizmo->ActiveOperation == ImGuizmoOperation_Scale) {
    if (hover_flags == ImGuizmoAxisFlags_ALL) {
      for (auto axis_idx = 0; axis_idx < 3; ++axis_idx) {
        RenderScaleTrail(axis_idx);
      }
    } else {
      RenderScaleTrail(GetAxisIdx(hover_flags, false));
    }
    RenderScaleInfo(gizmo->Scale);
  }
}
void BoundsScale(const float *bounds, const float *snap) {
  const ImGuizmoContext &g{GImGuizmo};
  const auto *gizmo = GetCurrentGizmo();

  BuildOuterPoints(bounds);
  BuildMidPoints();

  const auto model_view_proj =
    g.Camera.ViewProjectionMatrix * gizmo->SourceModelMatrix;

  ImGuizmoPlane hovered_plane_idx{-1};
  int32_t hovered_bound_idx{-1};
  auto hover_flags =
    FindHoveredBound(model_view_proj, hovered_plane_idx, hovered_bound_idx);
  IM_ASSERT(hovered_plane_idx < 3 && hovered_bound_idx < 4);

  bool held;
  if (BoundBehavior(hover_flags, hovered_plane_idx, hovered_bound_idx, &held)) {
    BeginBoundsScale();
  }

  glm::vec3 scale_info{0.0f};
  if (held) {
    scale_info = ContinueBoundsScale(bounds, snap);
  }
  if (!gizmo->ActiveManipulationFlags ||
      gizmo->ActiveOperation == ImGuizmoOperation_BoundsScale) {
    RenderBounds(model_view_proj, hover_flags, hovered_plane_idx,
                 hovered_bound_idx);
  }

  if (gizmo->ActiveOperation == ImGuizmoOperation_BoundsScale)
    RenderScaleInfo(scale_info);
}

}; // namespace ImGuizmo

//-----------------------------------------------------------------------------
// [SECTION] ImGuizmoContext METHODS
//-----------------------------------------------------------------------------

ImGuizmoContext::~ImGuizmoContext() {
  for (auto *gizmo : Gizmos) {
    delete gizmo;
  }

  Gizmos.clear();
  GizmosById.Clear();
  CurrentGizmo = nullptr;
}
float ImGuizmoContext::GetAspectRatio() const {
  return Viewport.GetWidth() / Viewport.GetHeight();
}

//-----------------------------------------------------------------------------
// [SECTION] ImGuizmoWidget METHODS
//-----------------------------------------------------------------------------

using namespace ImGuizmo;

void ImGuizmoWidget::Load(const glm::mat4 &model) {
  const ImGuizmoContext &g{GImGuizmo};

  SourceModelMatrix = model;
  if (Mode == ImGuizmoMode_Local) {
    ModelMatrix = SourceModelMatrix;
    for (auto axis_idx = 0; axis_idx < 3; ++axis_idx)
      ModelMatrix[axis_idx] = glm::normalize(ModelMatrix[axis_idx]);
  } else {
    ModelMatrix = glm::translate(glm::vec3{SourceModelMatrix[3]});
  }
  ModelViewProjMatrix = g.Camera.ViewProjectionMatrix * ModelMatrix;

  for (auto axis_idx = 0; axis_idx < 3; ++axis_idx)
    ModelScaleOrigin[axis_idx] = glm::length(SourceModelMatrix[axis_idx]);

  InversedModelMatrix = glm::inverse(ModelMatrix);
  const auto right_view_inverse =
    glm::vec3{InversedModelMatrix * glm::vec4{g.Camera.Right, 0.0f}};
  const auto right_length =
    GetSegmentLengthClipSpace(glm::vec3{0.0f}, right_view_inverse);
  ScreenFactor = glm::clamp(g.Style.GizmoScale / right_length, 0.5f, 5.0f);

  Origin = WorldToScreen(glm::vec3{0.0f}, ModelViewProjMatrix);

  const auto camera_space_position =
    glm::vec3{ModelViewProjMatrix * glm::vec4{glm::vec3{0.0f}, 1.0f}};
  Visible = (g.Camera.IsOrtho ? true : camera_space_position.z >= 0.0f) &&
            g.Viewport.Contains(Origin);
  if (!Visible) Hovered = false;

  RingRadius = g.Style.GizmoScale * g.Viewport.GetHeight();
}
float ImGuizmoWidget::CalculateAngleOnPlane() const {
  const ImGuizmoContext &g{GImGuizmo};

  const auto length = IntersectRayPlane(g.Ray, TranslationPlane);
  const auto local_pos = glm::normalize(
    g.Ray.Origin + g.Ray.Direction * length - glm::vec3{ModelMatrix[3]});

  const auto perpendicular = glm::normalize(
    glm::cross(RotationVectorSource, glm::vec3{TranslationPlane}));

  const auto acos_angle =
    glm::clamp(glm::dot(local_pos, RotationVectorSource), -1.0f, 1.0f);
  auto angle = glm::acos(acos_angle);
  angle *= (glm::dot(local_pos, perpendicular) < 0.0f) ? 1.0f : -1.0f;
  return angle;
}
