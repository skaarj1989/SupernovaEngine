#include "ShapeCreatorWidget.hpp"

#include "Jolt/Physics/PhysicsSettings.h" // cDefaultConvexRadius

#include "TransformInspector.hpp"
#include "ColliderInspector.hpp"

#include "physics/Conversion.hpp"
#include "physics/ShapeSerialization.hpp"

#include "ImGuiTitleBarMacro.hpp"
#include "IconsFontAwesome6.h"
#include "FileDialog.hpp"
#include "ImGuiPopups.hpp" // attachPopup
#include "ImGuiDragAndDrop.hpp"
#include "imgui_internal.h" // {Push/Pop}ItemFlag

#include "spdlog/spdlog.h"
#include "tracy/Tracy.hpp"

#include <array>

namespace {

struct ShapeControl {
  virtual ~ShapeControl() = default;

  [[nodiscard]] virtual bool canUse(const ShapeBuilder &) const {
    return true;
  };
  // @return true if the input is valid
  virtual bool inspect() = 0;
  virtual void apply(ShapeBuilder &) = 0;
};

struct Transformable {
  void inspect() {
    ImGui::Spacing();
    if (ImGui::Button("Reset")) {
      xf = {};
    }
    ImGui::SameLine();
    ImGui::SeparatorText("Transform");
    ImGui::Spacing();

    inspectPosition(xf.position);
    inspectOrientation(xf.orientation);
  }
  [[nodiscard]] std::optional<ShapeBuilder::Transform> getTransform() const {
    return xf.isIdentity() ? std::nullopt : std::make_optional(xf);
  }

  ShapeBuilder::Transform xf;
};

constexpr auto kMinSize = 0.01f;

struct SphereShapeControl final : ShapeControl, Transformable {
  bool inspect() override {
    ImGui::SeparatorText("Sphere");
    ImGui::Spacing();

    ImGui::DragFloat("radius", &radius, 0.1f, kMinSize, FLT_MAX);
    Transformable::inspect();
    return true;
  }
  void apply(ShapeBuilder &builder) override {
    builder.add(ShapeBuilder::makeSphere(radius), getTransform());
  }

  float radius{1.0f};
};
struct BoxShapeControl final : ShapeControl, Transformable {
  bool inspect() override {
    ImGui::SeparatorText("Box");
    ImGui::Spacing();

    ImGui::DragFloat3("halfExtents", halfExtents, 0.1f,
                      JPH::cDefaultConvexRadius + kMinSize, FLT_MAX);
    Transformable::inspect();
    return true;
  }
  void apply(ShapeBuilder &builder) override {
    builder.add(ShapeBuilder::makeBox(halfExtents), getTransform());
  }

  glm::vec3 halfExtents{1.0f};
};
struct CapsuleShapeControl final : ShapeControl, Transformable {
  bool inspect() override {
    ImGui::SeparatorText("Capsule");
    ImGui::Spacing();

    ImGui::DragFloat("halfHeight", &halfHeight, 0.1f,
                     JPH::cDefaultConvexRadius + kMinSize, FLT_MAX);
    ImGui::DragFloat("radius", &radius, 0.1f, kMinSize, FLT_MAX);
    Transformable::inspect();
    return true;
  }
  void apply(ShapeBuilder &builder) override {
    builder.add(ShapeBuilder::makeCapsule(halfHeight, radius), getTransform());
  }

  float halfHeight{1.0f};
  float radius{1.0f};
};

struct RotatedTranslatedShapeControl final : ShapeControl, Transformable {
  bool canUse(const ShapeBuilder &builder) const override {
    return builder.size() > 0;
  }
  bool inspect() override {
    Transformable::inspect();
    return !xf.isIdentity();
  }
  void apply(ShapeBuilder &builder) override {
    builder.rotateTranslate(xf.position, xf.orientation);
  }
};
struct ScaleShapeControl final : ShapeControl {
  bool canUse(const ShapeBuilder &builder) const override {
    return builder.size() > 0;
  }
  bool inspect() override {
    ImGui::DragFloat3("scale", scale, 0.1f, kMinSize, FLT_MAX);
    return scale != glm::vec3{1.0f};
  }
  void apply(ShapeBuilder &builder) override { builder.scale(scale); }

  glm::vec3 scale{1.0f};
};

void inspect(ShapeControl &control, ShapeBuilder &builder) {
  const auto validInput = control.inspect();

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  ImGui::BeginDisabled(!validInput);
  if (ImGui::Button(ICON_FA_TRASH " OK")) {
    control.apply(builder);
    ImGui::CloseCurrentPopup();
  }
  ImGui::EndDisabled();
  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_BAN " Cancel")) ImGui::CloseCurrentPopup();
}

// .first = MenuItem label
using ControlEntry = std::pair<const char *, std::unique_ptr<ShapeControl>>;

const auto g_controlEntries = std::array{
  ControlEntry{"Sphere", std::make_unique<SphereShapeControl>()},
  ControlEntry{"Box", std::make_unique<BoxShapeControl>()},
  ControlEntry{"Capsule", std::make_unique<CapsuleShapeControl>()},
  ControlEntry{"Scale", std::make_unique<ScaleShapeControl>()},
  ControlEntry{"RotateTranslate",
               std::make_unique<RotatedTranslatedShapeControl>()},
};

template <typename Func>
void importShapes(ShapeBuilder &builder, const std::filesystem::path &p,
                  Func func) {
  if (const auto s = readTriangleMesh(p); s) {
    builder.add(ShapeBuilder::makeCompound(func(*s)));
  } else {
    SPDLOG_ERROR("Could not read Mesh. '{}': {}",
                 os::FileSystem::relativeToRoot(p)->generic_string(),
                 s.error());
  }
}
void loadShapes(ShapeBuilder &builder, const std::filesystem::path &p) {
  if (auto s = loadShape(p); s) {
    builder.set(std::move(*s));
  } else {
    SPDLOG_ERROR("Could not load Shape. '{}': {}",
                 os::FileSystem::relativeToRoot(p)->generic_string(),
                 s.error());
  }
}

} // namespace

//
// ShapeCreatorWidget class:
//

void ShapeCreatorWidget::show(const char *name, bool *open) {
  constexpr auto kLoadShapeActionId = MAKE_TITLE_BAR(ICON_FA_UPLOAD, " Load");
  constexpr auto kSaveShapeActionId =
    MAKE_TITLE_BAR(ICON_FA_FLOPPY_DISK, " Save");
  constexpr auto kImportMeshAsTriangleMeshActionId =
    MAKE_TITLE_BAR(ICON_FA_UPLOAD, " Mesh->TriangleMesh");
  constexpr auto kImportMeshAsConvexHullActionId =
    MAKE_TITLE_BAR(ICON_FA_UPLOAD, " Mesh->ConvexHull");

  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    ZoneScopedN("ShapeCreatorWidget");
    std::optional<const char *> action;
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu(ICON_FA_FILE " File")) {
        if (ImGui::MenuItem(ICON_FA_UPLOAD " Load")) {
          action = kLoadShapeActionId;
        }
        if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK " Save", nullptr, nullptr,
                            m_builder.size() > 0)) {
          action = kSaveShapeActionId;
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu(ICON_FA_SHAPES " Shape")) {
        ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
        for (const auto &[label, control] : g_controlEntries) {
          ImGui::MenuItem(label, nullptr, nullptr, control->canUse(m_builder));
          attachPopup(nullptr, ImGuiMouseButton_Left,
                      [this, &control] { inspect(*control, m_builder); });
        }
        ImGui::PopItemFlag();

        if (ImGui::BeginMenu(ICON_FA_FILE_IMPORT " Import Mesh ...")) {
          if (ImGui::MenuItem("TriangleMesh")) {
            action = kImportMeshAsTriangleMeshActionId;
          }
          if (ImGui::MenuItem("ConvexHull")) {
            action = kImportMeshAsConvexHullActionId;
          }

          ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem(ICON_FA_ERASER " Clear", nullptr, nullptr,
                            m_builder.size() > 0)) {
          m_builder.reset();
        }
        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }

    if (action) ImGui::OpenPopup(*action);

    const auto &rootDir = os::FileSystem::getRoot();
    static auto currentDir = rootDir;

    if (const auto p = showFileDialog(kImportMeshAsTriangleMeshActionId,
                                      {.dir = currentDir, .barrier = rootDir});
        p) {
      importShapes(m_builder, *p, ShapeBuilder::toMesh);
    }
    if (const auto p = showFileDialog(kImportMeshAsConvexHullActionId,
                                      {.dir = currentDir, .barrier = rootDir});
        p) {
      importShapes(m_builder, *p, [](const auto &mesh) {
        return ShapeBuilder::toConvexHull(mesh);
      });
    }

    if (const auto p = showFileDialog(kLoadShapeActionId,
                                      {.dir = currentDir, .barrier = rootDir});
        p) {
      loadShapes(m_builder, *p);
    }
    if (const auto p =
          showFileDialog(kSaveShapeActionId,
                         {
                           .dir = currentDir,
                           .barrier = rootDir,
                           .flags = FileDialogFlags_AskOverwrite |
                                    FileDialogFlags_CreateDirectoryButton,
                         });
        p) {
      if (!saveShape(*p, m_builder.build())) {
        SPDLOG_ERROR("Could not save Shape: '{}'",
                     os::FileSystem::relativeToRoot(*p)->generic_string());
      }
    }

    // ---

    if (m_builder.size() > 0) {
      if (ImGui::BeginChild(IM_UNIQUE_ID)) {
        print(m_builder.build().Get());
        ImGui::EndChild();
      }
    } else {
      ImGui::BeginDisabled(true);
      ImGui::Button("(empty)", ImGui::GetContentRegionAvail());
      ImGui::EndDisabled();
    }
    onDropTarget(kImGuiPayloadTypeFile, [this](const ImGuiPayload *payload) {
      loadShapes(m_builder, ImGui::ExtractPath(*payload));
    });
  }
  ImGui::End();
}
