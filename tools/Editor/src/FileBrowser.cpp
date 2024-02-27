#include "FileBrowser.hpp"
#include "os/FileSystem.hpp"
#include "StringUtility.hpp" // contains

#include "IconsFontAwesome6.h"
#include "ImGuiPopups.hpp" // attachPopup
#include "ImGuiDragAndDrop.hpp"
#include "imgui_internal.h" // {Push/Pop}ItemFlag
#include "imgui_stdlib.h"   // InputTextWithHint

#include "spdlog/spdlog.h"

namespace {

[[nodiscard]] auto getDirectory(const std::filesystem::directory_entry &e) {
  return e.is_directory() ? e.path() : e.path().parent_path();
}

} // namespace

void FileBrowserWidget::show(const char *name, bool *open) {
  ZoneScopedN("FileBrowserWidget");
  if (ImGui::Begin(name, open)) {
    if (ImGui::InputTextWithHint(IM_UNIQUE_ID, "Filter", &m_pattern)) {
      m_selected.clear();
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(ICON_FA_ERASER)) m_pattern.clear();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    _viewDirectory(os::FileSystem::getRoot());
  }
  ImGui::End();
}

//
// (private):
//

#define INVOKE_FILESYSTEM_FN(fn, ...)                                          \
  do {                                                                         \
    std::error_code ec;                                                        \
    std::filesystem::fn(__VA_ARGS__, ec);                                      \
    if (ec) {                                                                  \
      SPDLOG_ERROR(ec.message());                                              \
    }                                                                          \
  } while (false)

void FileBrowserWidget::_viewDirectory(const std::filesystem::path &dir) {
  ZoneScopedN("FileBrowserWidget::Dir");

  std::error_code ec;
  for (auto &&entry : std::filesystem::directory_iterator{dir, ec}) {
    auto name = entry.path().filename().string();
    if (!m_pattern.empty() && !contains(name, m_pattern)) continue;

    const auto icon = entry.is_directory() ? ICON_FA_FOLDER : ICON_FA_FILE;
    const auto label = std::format("{} {}", icon, name);
    auto isSelected = m_selected.contains(entry);

    auto opened = false;

    if (entry.is_directory()) {
      constexpr auto kFlags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
      opened = ImGui::TreeNodeEx(
        label.c_str(), kFlags | (isSelected ? ImGuiTreeNodeFlags_Selected
                                            : ImGuiTreeNodeFlags_None));
    } else {
      ImGui::Selectable(label.c_str(), &isSelected);
    }
    // Entry selection (CTRL for multiple entries).
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
      isSelected ? m_selected.erase(entry) : _select(entry);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      _select(entry);
    }

    // Drag'n'Drop to move an entry between directories/other widgets.
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      const auto payloadType = entry.is_directory() ? kImGuiPayloadTypeDirectory
                                                    : kImGuiPayloadTypeFile;
      const auto strPath =
        os::FileSystem::relativeToRoot(entry.path())->generic_string();
      ImGui::SetDragDropPayload(payloadType, strPath);
      ImGui::Text("%s %s", icon, strPath.c_str());
      ImGui::EndDragDropSource();
    }
    onDropTarget(kImGuiPayloadTypeFile, [this, &entry](auto *payload) {
      const auto source = ImGui::ExtractPath(*payload);
      const auto target = getDirectory(entry) / source.filename();
      if (source != target) {
        INVOKE_FILESYSTEM_FN(rename, source, target);
        m_selected.clear();
      }
    });

    if (m_selected.contains(entry)) {
      attachPopup(nullptr, ImGuiMouseButton_Right, [this, &entry, &name] {
        if (m_selected.size() == 1) {
          ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
          ImGui::MenuItem(ICON_FA_FOLDER_PLUS " New Folder");
          ImGui::PopItemFlag();
          attachPopup(nullptr, ImGuiMouseButton_Left, [&entry] {
            std::string dirname{"New folder"};
            ImGui::SetNextItemWidth(150);
            if (ImGui::InputTextWithHint(
                  IM_UNIQUE_ID, "Name", &dirname,
                  ImGuiInputTextFlags_CallbackCharFilter |
                    ImGuiInputTextFlags_EnterReturnsTrue,
                  blockFileSystemForbiddenCharacters)) {
              if (!dirname.empty()) {
                INVOKE_FILESYSTEM_FN(create_directory,
                                     getDirectory(entry) / dirname);
                ImGui::CloseCurrentPopup();
              }
            }
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FA_XMARK)) ImGui::CloseCurrentPopup();
          });

          if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open in Explorer")) {
            std::system(
              std::format("explorer {}", getDirectory(entry).string()).c_str());
          }

          ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
          ImGui::MenuItem(ICON_FA_PEN " Rename");
          ImGui::PopItemFlag();
          attachPopup(nullptr, ImGuiMouseButton_Left, [this, &entry, &name] {
            ImGui::SetNextItemWidth(150);
            if (ImGui::InputTextWithHint(
                  IM_UNIQUE_ID, "Name", &name,
                  ImGuiInputTextFlags_CallbackCharFilter |
                    ImGuiInputTextFlags_EnterReturnsTrue,
                  blockFileSystemForbiddenCharacters)) {
              if (!name.empty()) {
                const auto target = entry.path().parent_path() / name;
                INVOKE_FILESYSTEM_FN(rename, entry.path(), target);
                m_selected.clear();
              }
            }
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FA_XMARK)) ImGui::CloseCurrentPopup();
          });

          ImGui::Separator();
        }
        ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
        ImGui::MenuItem(ICON_FA_TRASH " Remove");
        ImGui::PopItemFlag();
        attachPopup(nullptr, ImGuiMouseButton_Left, [this] {
          ImGui::Text("Do you really want to remove the selected file(s)?");
          ImGui::Spacing();
          ImGui::Separator();
          ImGui::Spacing();
          if (ImGui::Button(ICON_FA_TRASH " Yes")) {
            std::ranges::for_each(m_selected, [](const auto &e) {
              INVOKE_FILESYSTEM_FN(remove_all, e);
            });
            m_selected.clear();

            ImGui::CloseCurrentPopup();
          }
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_BAN " No")) ImGui::CloseCurrentPopup();
        });
      });
    }

    if (opened) {
      if (std::filesystem::is_directory(entry)) {
        ImGui::Indent();
        _viewDirectory(entry.path());
        ImGui::Unindent();
      }
      ImGui::TreePop();
    }
  }
}

#undef INVOKE_FILESYSTEM_FN

std::size_t
FileBrowserWidget::_select(const std::filesystem::directory_entry &e) {
  if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
    m_selected.insert(e);
  } else {
    m_selected = {e};
  }
  return m_selected.size();
}
