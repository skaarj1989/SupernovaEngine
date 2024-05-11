#pragma once

#include "WidgetWindow.hpp"
#include <filesystem>
#include <unordered_set>

namespace std {

template <> struct hash<filesystem::directory_entry> {
  auto operator()(const filesystem::directory_entry &v) const noexcept {
    return std::filesystem::hash_value(v);
  }
};

} // namespace std

class FileBrowserWidget final : public WidgetWindow {
public:
  void show(const char *name, bool *open) override;

private:
  void _viewDirectory(const std::filesystem::path &);
  std::size_t _select(const std::filesystem::directory_entry &);

private:
  std::string m_pattern;

  std::unordered_set<std::filesystem::directory_entry> m_selected;
};
