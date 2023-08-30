#pragma once

#include "imgui.h"

enum ImGuizmoCol_ {
  ImGuizmoCol_Text,
  ImGuizmoCol_TextShadow,

  ImGuizmoCol_Inactive,
  ImGuizmoCol_Hovered,

  ImGuizmoCol_SpecialMove,

  ImGuizmoCol_AxisX,
  ImGuizmoCol_AxisY,
  ImGuizmoCol_AxisZ,

  ImGuizmoCol_PlaneYZ,
  ImGuizmoCol_PlaneZX,
  ImGuizmoCol_PlaneXY,

  ImGuizmoCol_BoundAnchor,

  ImGuizmoCol_COUNT
};
using ImGuizmoCol = int32_t;

struct ImGuizmoStyle {
  float GizmoScale{0.1f};
  float RotationRingThickness{3.5f};

  float Alpha{1.0f};
  ImVec4 Colors[ImGuizmoCol_COUNT];

  IMGUI_API ImGuizmoStyle();
};

enum ImGuizmoMode_ {
  ImGuizmoMode_World, // Transform tools aligned to world grid.
  ImGuizmoMode_Local, // Transform tools aligned to rotation of model matrix.

  ImGuizmoMode_COUNT
};
using ImGuizmoMode = int32_t;

enum ImGuizmoOperation_ {
  ImGuizmoOperation_None = 0,

  ImGuizmoOperation_Translate,
  ImGuizmoOperation_Rotate,
  ImGuizmoOperation_Scale,
  ImGuizmoOperation_BoundsScale,
};
using ImGuizmoOperation = int32_t;

enum ImGuizmoAxisFlags_ {
  ImGuizmoAxisFlags_None = 0,

  ImGuizmoAxisFlags_X = 1 << 0,
  ImGuizmoAxisFlags_Y = 1 << 1,
  ImGuizmoAxisFlags_Z = 1 << 2,

  ImGuizmoAxisFlags_YZ = ImGuizmoAxisFlags_Y | ImGuizmoAxisFlags_Z,
  ImGuizmoAxisFlags_ZX = ImGuizmoAxisFlags_Z | ImGuizmoAxisFlags_X,
  ImGuizmoAxisFlags_XY = ImGuizmoAxisFlags_X | ImGuizmoAxisFlags_Y,

  ImGuizmoAxisFlags_ALL =
    ImGuizmoAxisFlags_X | ImGuizmoAxisFlags_Y | ImGuizmoAxisFlags_Z
};
using ImGuizmoAxisFlags = uint32_t;

enum ImGuizmoConfigFlags_ {
  ImGuizmoConfigFlags_None = 0,

  // Render only active manipulation.
  ImGuizmoConfigFlags_CloakOnManipulate = 1 << 0,
  // Hides locked axes instead of drawing as inactive.
  ImGuizmoConfigFlags_HideLocked = 1 << 1,
  // Cancel active manipulation on RMB.
  ImGuizmoConfigFlags_HasReversing = 1 << 2
};
using ImGuizmoConfigFlags = uint32_t;

namespace ImGuizmo {

IMGUI_API void PrintContext();

IMGUI_API bool IsUsing();
IMGUI_API bool IsHovered();

IMGUI_API ImGuizmoStyle &GetStyle();
IMGUI_API void StyleColorsClassic(ImGuizmoStyle *dst = nullptr);
IMGUI_API void StyleColorsBlender(ImGuizmoStyle *dst = nullptr);
IMGUI_API void StyleColorsUnreal(ImGuizmoStyle *dst = nullptr);

IMGUI_API void ShowStyleEditor(ImGuizmoStyle *ref = nullptr);
IMGUI_API bool ShowStyleSelector(const char *label);

IMGUI_API void SetConfigFlags(ImGuizmoConfigFlags flags);

IMGUI_API void SetDrawlist(ImDrawList * = nullptr);

/**
 * @param [in] view_matrix Camera view matrix (column-major).
 * @param [in] projection_matrix Camera projection matrix (column-major).
 */
IMGUI_API void SetCamera(const float *view_matrix,
                         const float *projection_matrix, bool invert_y = false,
                         bool is_ortho = false);

/**
 * @note Convenience method.
 * @param [in] snap shared between all operations.
 * @return same as End().
 */
IMGUI_API bool Manipulate(ImGuizmoMode, ImGuizmoOperation, float *model_matrix,
                          const float *snap = nullptr);

/**
 * @param [inout] model_matrix Model matrix (column-major).
 * @return true if gizmo is visible.
 */
IMGUI_API bool Begin(ImGuizmoMode, float *model_matrix,
                     ImGuizmoAxisFlags locked_axes = ImGuizmoAxisFlags_None);
/**
 * Saves result to locked model matrix if manipulation has been made between
 * Begin/End.
 * @return true if matrix has been updated.
 */
IMGUI_API bool End(float *delta_matrix = nullptr);

/** @param [in] snap */
IMGUI_API void Translate(const float *snap = nullptr);
/** @param [in] snap */
IMGUI_API void Rotate(const float *snap = nullptr);
/** @param [in] snap  */
IMGUI_API void Scale(const float *snap = nullptr);

/**
 * @param [in] bounds array[6]
 * @param [in] snap
 */
IMGUI_API void BoundsScale(const float *bounds, const float *snap = nullptr);

} // namespace ImGuizmo
