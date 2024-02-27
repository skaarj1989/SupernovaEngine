#include "LoggerWidget.hpp"

#include "imgui.h"
#include "spdlog/sinks/base_sink.h"

namespace {

#define FLATTEN_MULTI_LINE_ENTRIES 1
#define USE_CLIPPER 1

using Entry = LoggerWidget::Entry;

template <typename Mutex>
class CustomSink final : public spdlog::sinks::base_sink<Mutex> {
public:
  explicit CustomSink(std::vector<Entry> &e) : m_entries{&e} {}

private:
  void sink_it_(const spdlog::details::log_msg &msg) override {
#if FLATTEN_MULTI_LINE_ENTRIES
    constexpr auto kNextLineDelimiter = '\n';

    std::istringstream iss{fmt::to_string(msg.payload)};

    auto firstLine = true;
    std::string line;
    while (std::getline(iss, line, kNextLineDelimiter)) {
      if (line.empty()) continue;

      Entry entry{.message = line};
      if (firstLine) {
        entry.prefix = {
          .time = std::format("{}", msg.time),
          .level = msg.level,
        };
        firstLine = false;
      }
      m_entries->push_back(std::move(entry));
    }
#else
    m_entries->push_back(Entry{
      .prefix =
        Entry::Prefix{
          .time = std::format("{}", msg.time),
          .level = msg.level,
        },
      .message = fmt::to_string(msg.payload),
    });
#endif
  }

  void flush_() override {}

private:
  std::vector<Entry> *m_entries{nullptr};
};

using CustomSinkST = CustomSink<spdlog::details::null_mutex>;

[[nodiscard]] std::pair<const char *, ImVec4>
statLevel(spdlog::level::level_enum level) {
  switch (level) {
    using enum spdlog::level::level_enum;

  case info:
    return {"info", ImVec4{0, 1, 0, 1}};
  case warn:
    return {"warn", ImVec4{1, 1, 0, 1}};
  case err:
    return {"error", ImVec4{1, 0, 0, 1}};
  case critical:
    return {"critical", ImVec4{1, 0, 0, 1}};
  }

  return {"unknown", ImVec4{1, 1, 1, 1}};
}

#if USE_CLIPPER
[[nodiscard]] auto print(const Entry &entry,
                         std::optional<float> lastCursorPosX) {
  if (entry.prefix) {
    auto str = std::format("[{}]", entry.prefix->time);
    auto textWidth = ImGui::CalcTextSize(str.c_str()).x;
    ImGui::Text(str.c_str());

    auto [levelStr, levelColor] = statLevel(entry.prefix->level);
    str = std::format("[{}]", levelStr);
    textWidth += ImGui::CalcTextSize(str.c_str()).x;

    ImGui::SameLine();
    ImGui::TextColored(levelColor, str.c_str());

    lastCursorPosX = textWidth + 20.0f;

    ImGui::SameLine();
  } else {
    // ... justify to last entry.
    if (lastCursorPosX) ImGui::SetCursorPosX(*lastCursorPosX);
  }

  ImGui::Text(entry.message.c_str());

  return lastCursorPosX;
}
#else
void print(const Entry &entry) {
  if (entry.prefix) {
    ImGui::Text("[%s]", entry.prefix->time.c_str());

    auto [levelStr, levelColor] = statLevel(entry.prefix->level);
    ImGui::SameLine();
    ImGui::TextColored(levelColor, "[%s]", levelStr);

    ImGui::SameLine();
  }
  ImGui::Text(entry.message.c_str());
}
#endif

} // namespace

//
// LoggerWidget class:
//

LoggerWidget::LoggerWidget(spdlog::logger &logger) : m_logger{&logger} {
  m_sink = std::make_shared<CustomSinkST>(m_entries);
  m_logger->sinks().push_back(m_sink);
}
LoggerWidget::~LoggerWidget() { std::erase(m_logger->sinks(), m_sink); }

void LoggerWidget::show(const char *name, bool *open) {
  ZoneScopedN("LoggerWidget");
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    if (ImGui::BeginMenuBar()) {
      if (ImGui::Button("Clear")) {
        m_entries.clear();
      }
      ImGui::Checkbox("Auto-scroll", &m_autoScroll);
      ImGui::EndMenuBar();
    }

#if USE_CLIPPER
    ImGuiListClipper clipper;
    clipper.Begin(m_entries.size());
    while (clipper.Step()) {
      std::optional<float> lastCursorPosX;
      for (auto i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
        lastCursorPosX = print(m_entries[i], lastCursorPosX);
      }
    }
    clipper.End();
#else
    std::ranges::for_each(m_entries, print);
#endif

    if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
      ImGui::SetScrollHereY(1.0f);
  }
  ImGui::End();
}
