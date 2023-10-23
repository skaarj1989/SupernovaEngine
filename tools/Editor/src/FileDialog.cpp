#include "FileDialog.hpp"
#include "StringUtility.hpp" // formatBytes

#include "ImGuiTitleBarMacro.hpp"
#include "ImGuiModal.hpp"
#include "imgui_stdlib.h" // InputText

#include "spdlog/spdlog.h"

namespace {

[[nodiscard]] std::string formatFileClock(std::filesystem::file_time_type t) {
#if 1
  // https://stackoverflow.com/questions/56788745/how-to-convert-stdfilesystemfile-time-type-to-a-string-using-gcc-9
  using namespace std::chrono;
  const auto sctp = time_point_cast<system_clock::duration>(
    t - decltype(t)::clock::now() + system_clock::now());
  const auto tt = system_clock::to_time_t(sctp);
  const auto *lt = std::localtime(&tt);
  assert(lt);
  std::ostringstream oss;
  oss << std::put_time(lt, "%Y-%m-%d %H:%M");
  return oss.str();
#else
  // https://developercommunity.visualstudio.com/t/Reported-memory-leak-when-converting-fil/1467739#T-N1516289
  return std::format("{}", t);
#endif
}

[[nodiscard]] auto createNewFolderModal(const char *id) {
  std::optional<std::string> result;
  if (ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    static std::string name;
    ImGui::InputTextWithHint(IM_UNIQUE_ID, "Name", &name,
                             ImGuiInputTextFlags_CallbackCharFilter,
                             blockFileSystemForbiddenCharacters);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginDisabled(name.empty());
    if (ImGui::Button(ICON_FA_CHECK " OK")) {
      result.emplace(std::move(name));
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BAN " Cancel")) ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
  }
  return result;
}

enum class EntryAction {
  None = 0,
  Click,
  DblClick,
};

[[nodiscard]] auto pickIcon(const std::filesystem::directory_entry &entry) {
  switch (entry.status().type()) {
    using enum std::filesystem::file_type;

  case directory:
    return ICON_FA_FOLDER;
  case regular:
    return ICON_FA_FILE;
  }
  return ICON_FA_QUESTION;
}
[[nodiscard]] auto makeLabel(const std::filesystem::directory_entry &entry) {
  return std::format("{} {}", pickIcon(entry),
                     entry.path().filename().string());
}

[[nodiscard]] auto
selectableEntry(const std::filesystem::directory_entry &entry,
                const std::string &userLabel) {
  EntryAction action{EntryAction::None};

  ImGui::TableNextRow();

  ImGui::TableSetColumnIndex(0);
  ImGui::AlignTextToFramePadding();
  constexpr auto kFlags = ImGuiSelectableFlags_SpanAllColumns |
                          ImGuiSelectableFlags_AllowDoubleClick |
                          ImGuiSelectableFlags_DontClosePopups;
  const auto selected = entry.path().filename() == userLabel;
  if (ImGui::Selectable(makeLabel(entry).c_str(), selected, kFlags)) {
    action = EntryAction::Click;
  }
  if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) &&
      ImGui::IsItemHovered()) {
    action = EntryAction::DblClick;
  }

  ImGui::TableSetColumnIndex(1);
  ImGui::Text(formatFileClock(entry.last_write_time()).c_str());

  ImGui::TableSetColumnIndex(2);
  std::error_code e;
  const auto size = entry.file_size(e);
  ImGui::Text("%s", !e ? formatBytes(size).c_str() : "--");

  return action;
}

} // namespace

std::optional<std::filesystem::path>
showFileDialog(const char *name, const FileDialogSettings &settings) {
  assert(!settings.forceExtension ||
         !(settings.flags & FileDialogFlags_DirectoryOnly));

  if (!std::filesystem::is_directory(settings.dir)) {
    assert(false);
    return std::nullopt;
  }
  std::optional<std::filesystem::path> result;

  ImGui::CenterNextWindow();
  if (ImGui::BeginPopupModal(name, nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    // -- Optional "up" directory button + current path:

    if (ImGui::Button(ICON_FA_ARROW_UP)) {
      if (!settings.barrier || *settings.barrier != settings.dir)
        settings.dir = settings.dir.parent_path();
    }
    ImGui::SameLine();
    if (settings.barrier) {
      ImGui::Text(R"(//%s)", settings.dir.lexically_relative(*settings.barrier)
                               .generic_string()
                               .c_str());
    } else {
      ImGui::Text(settings.dir.generic_string().c_str());
    }

    // -- Optional "create directory" button:

    if (settings.flags & FileDialogFlags_CreateDirectoryButton) {
      constexpr auto kNewFolderPopupId =
        MAKE_TITLE_BAR(ICON_FA_FOLDER, "Create directory");

      if (ImGui::Button(ICON_FA_FOLDER_PLUS " New folder"))
        ImGui::OpenPopup(kNewFolderPopupId);

      if (auto p = createNewFolderModal(kNewFolderPopupId); p) {
        if (std::error_code ec;
            !std::filesystem::create_directory(settings.dir / *p, ec)) {
          SPDLOG_ERROR(ec.message());
        }
      }
    }

    // -- Directory entries:

    static std::string label;

    constexpr auto kTableFlags = ImGuiTableFlags_Borders |
                                 ImGuiTableFlags_RowBg |
                                 ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable(IM_UNIQUE_ID, 3, kTableFlags)) {
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
      ImGui::TableSetupColumn("Date modified");
      ImGui::TableSetupColumn("Size");
      ImGui::TableHeadersRow();

      auto numEntries = 0;
      for (auto &&entry : std::filesystem::directory_iterator{settings.dir}) {
        if (settings.flags & FileDialogFlags_DirectoryOnly) {
          if (!entry.is_directory()) continue;
        }
        if (!entry.is_directory() && settings.entryFilter) {
          if (const auto pass = std::invoke(settings.entryFilter, entry); !pass)
            continue;
        }

        switch (selectableEntry(entry, label)) {
        case EntryAction::DblClick:
          if (entry.is_directory()) {
            settings.dir /= entry;
            label.clear();
          }
          break;
        case EntryAction::Click:
          label = entry.path().filename().string();
          break;
        }
        ++numEntries;
      }
      if (numEntries == 0) {
        ImGui::TableNextColumn();
        ImGui::Text("(No items)");
      }
      ImGui::EndTable();
    }

    ImGui::Frame([&settings, w = ImGui::CalcItemWidth()] {
      ImGui::SetNextItemWidth(w);
      ImGui::InputTextWithHint(IM_UNIQUE_ID, "Name", &label,
                               ImGuiInputTextFlags_CallbackCharFilter,
                               blockFileSystemForbiddenCharacters);
      if (settings.forceExtension) {
        ImGui::SameLine();
        ImGui::Text(settings.forceExtension->data());
      }
    });

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    constexpr auto kConfirmOverwrite = MAKE_WARNING("Overwrite?");

    if (ImGui::Button(ICON_FA_CHECK " OK", {120, 0})) {
      if (!label.empty()) {
        if (auto p = settings.dir / label; std::filesystem::is_directory(p)) {
          settings.dir = std::move(p);
          label.clear();
        } else {
          if (settings.forceExtension) {
            assert(!settings.forceExtension->empty());
            p.replace_extension(*settings.forceExtension);
          }
          if (std::filesystem::exists(p) &&
              settings.flags & FileDialogFlags_AskOverwrite) {
            ImGui::OpenPopup(kConfirmOverwrite);
          } else {
            result.emplace(std::move(p));
            label.clear();
            ImGui::CloseCurrentPopup();
          }
        }
      } else {
        if (settings.flags & FileDialogFlags_DirectoryOnly) {
          result.emplace(settings.dir);
        }
      }
    }

    if (settings.flags & FileDialogFlags_AskOverwrite) {
      if (const auto button =
            showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
              kConfirmOverwrite, "Overwrite this file?");
          button && *button == ModalButton::Yes) {
        assert(!label.empty());
        result.emplace(settings.dir / label);
      }
    }

    if (result) ImGui::CloseCurrentPopup();

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BAN " Cancel", {120, 0}))
      ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
  }
  return result;
}
