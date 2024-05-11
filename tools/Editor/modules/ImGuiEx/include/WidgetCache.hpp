#pragma once

#include "WidgetWindow.hpp"
#include "imgui.h"
#include <memory>
#include <typeindex>

template <typename Config> class WidgetCache {
public:
  template <class T, typename... Args>
  void add(const char *label, Config config, Args &&...args) {
    m_widgets.emplace_back(typeid(T),
                           Entry{
                             label,
                             std::move(config),
                             std::make_unique<T>(std::forward<Args>(args)...),
                           });
  }
  template <class T> [[nodiscard]] Config &getConfig() {
    return _getEntry<T>().config;
  }
  template <class T> [[nodiscard]] T &get() {
    return *static_cast<T *>(_getEntry<T>().widget.get());
  }

  auto begin() { return m_widgets.begin(); }
  auto end() { return m_widgets.end(); }
  auto cbegin() const { return m_widgets.cbegin(); }
  auto cend() const { return m_widgets.cend(); }
  auto begin() const { return cbegin(); }
  auto end() const { return cend(); }

private:
  template <class T>
  [[nodiscard]] auto &_getEntry()
    requires(!std::is_same_v<T, SimpleWidgetWindow>)
  {
    auto it = std::ranges::find_if(
      begin(), end(), [](const auto &p) { return p.first == typeid(T); });
    assert(it != end());
    return it->second;
  }

private:
  struct Entry {
    const char *label{nullptr};
    Config config;
    std::unique_ptr<WidgetWindow> widget;
  };
  std::vector<std::pair<std::type_index, Entry>> m_widgets;
};

template <class T> void show(T &widgetCache) {
  for (auto &[_, entry] : widgetCache) {
    auto &config = entry.config;
    if (config.open) {
      entry.widget->show(config.name, &config.open);
    }
  }
}

namespace ImGui {

template <class T> void MenuItems(T &widgetCache) {
  for (auto &[_, entry] : widgetCache) {
    MenuItem(entry.label, nullptr, &entry.config.open);
  }
}

} // namespace ImGui
