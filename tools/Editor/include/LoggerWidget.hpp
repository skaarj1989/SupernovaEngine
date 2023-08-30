#pragma once

#include "WidgetWindow.hpp"
#include "spdlog/logger.h"
#include <vector>

class LoggerWidget final : public WidgetWindow {
public:
  struct Entry {
    struct Prefix {
      std::string time;
      spdlog::level::level_enum level;
    };
    std::optional<Prefix> prefix;
    std::string message;
  };

  explicit LoggerWidget(spdlog::logger &);
  ~LoggerWidget() override;

  void show(const char *name, bool *open) override;

private:
  spdlog::logger *m_logger{nullptr};
  spdlog::sink_ptr m_sink;

  std::vector<Entry> m_entries;
  bool m_autoScroll{true};
};
