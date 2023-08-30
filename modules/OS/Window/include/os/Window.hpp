#pragma once

#include "WindowEvents.hpp"
#include "entt/signal/emitter.hpp"

#if WIN32
#  include <Windows.h>
#elif defined __linux
#  error Not implemented!
#endif

namespace os {

// NOTE: Only one listener per window (unfortunately).
class Window final : private entt::emitter<Window> {
  friend class entt::emitter<Window>;

  struct PlatformData {
#if WIN32
    HINSTANCE hInstance{nullptr};
    HWND hWnd{nullptr};
    HDC hDC{nullptr};
#elif defined __linux
    // TODO ...
#endif
  };

public:
  Window() = default;
  Window(const Window &) = delete;
  Window(Window &&) noexcept;
  ~Window() override;

  Window &operator=(const Window &) = delete;
  Window &operator=(Window &&) noexcept;

  // @return true if window is open/valid.
  [[nodiscard]] explicit operator bool() const;

  // ---

  using entt::emitter<Window>::on;
  using entt::emitter<Window>::erase;

  // ---

  // @param Top-left corner.
  Window &setPosition(glm::ivec2);
  // @param {width, height}.
  Window &setSize(glm::ivec2);
  Window &setAlpha(float);

  Window &setCaption(const std::string_view);

  // ---

  // @return Top-left corner.
  [[nodiscard]] glm::ivec2 getPosition() const;
  // @return {width, height}
  [[nodiscard]] glm::ivec2 getSize() const;
  // @return {width, height} without border (menu bar, etc.)
  [[nodiscard]] glm::uvec2 getClientSize() const;
  // @return Text in the title bar.
  [[nodiscard]] std::string_view getCaption() const;

  // ---

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool isMinimized() const;
  [[nodiscard]] bool hasFocus() const;

  // ---

  Window &show();
  Window &hide();

  Window &minimize();
  Window &maximize();

  Window &focus();

  void close();

  // ---

  // @return OS internal data.
  [[nodiscard]] const PlatformData &getNativeData() const;

  // ---

  class Builder {
  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder operator=(const Builder &) = delete;
    Builder operator=(Builder &&) noexcept = delete;

    Builder &setParent(const Window &);
    Builder &setPosition(glm::ivec2);
    Builder &setSize(glm::ivec2);
    Builder &setCaption(const std::string_view);

    [[nodiscard]] Window build() const;

  private:
    const Window *m_parent{nullptr};
    glm::ivec2 m_position{};
    glm::ivec2 m_size{};
    std::string m_caption;
  };

private:
#if WIN32
  Window(HWND parent, RECT, DWORD style, DWORD exStyle,
         const std::string_view caption);

  static LRESULT CALLBACK _wndProcRouter(HWND, UINT uMsg, WPARAM, LPARAM);
  LRESULT CALLBACK _wndProc(HWND, UINT uMsg, WPARAM, LPARAM);
#elif defined __linux
  // TODO ...
#endif

  void _destroy();

private:
  PlatformData m_native;
#if WIN32
  DWORD m_style{0};
  DWORD m_exStyle{0};

  RECT m_bounds{};     // In screen coordinates (includes border/titlebar area).
  SIZE m_clientSize{}; // Lower-right coord.
#elif defined __linux
  // TODO ...
#endif

  std::string m_caption;
  float m_alpha{0.0f};
};

// Polls events for a given window, or all (thread) windows if nullptr is given.
// @return Event count.
uint32_t pollEvents(const Window * = nullptr);
[[nodiscard]] float getAspectRatio(const Window &);

// Moves a given window to the center of a primary monitor.
void center(Window &);
glm::uvec2 getCenter(const Window &);

} // namespace os
