#pragma once

#include "WindowEvents.hpp"
#include "entt/signal/emitter.hpp"
#include <optional>

#if WIN32
#  include <Windows.h>
#elif defined __linux
#  include <xcb/xcb.h>
#endif

namespace os {

// NOTE: Only one listener per event in a window (unfortunately).
class Window final : private entt::emitter<Window> {
  friend class entt::emitter<Window>;

  friend uint32_t pollEvents();

  struct PlatformData {
#if WIN32
    HINSTANCE hInstance{nullptr};
    HWND hWnd{nullptr};
    HDC hDC{nullptr};
#elif defined __linux
    xcb_connection_t *connection{nullptr};
    xcb_window_t id{XCB_WINDOW_NONE};
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

  using Position = glm::ivec2;
  using Extent = glm::ivec2;
  using ClientSize = glm::uvec2;
  enum class State { Windowed = 0, Minimized, Maximized };

  // ---

  // @param Top-left corner (with non-client area).
  Window &setPosition(const Position);
  // @param {width, height} (with non-client area).
  Window &setExtent(const Extent);
  Window &setAlpha(float);

  Window &setCaption(const std::string_view);

  // ---

  enum class Area {
    Client,
    Absolute, // With decorations (borders, title bar).
  };

  // @return Top-left corner.
  [[nodiscard]] Position getPosition(const Area = Area::Client) const;
  // @return {width, height}
  [[nodiscard]] Extent getExtent(const Area = Area::Client) const;
  // @return Opacity.
  [[nodiscard]] float getAlpha() const;

  // @return Text in the title bar.
  [[nodiscard]] std::string_view getCaption() const;

  // ---

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] State getState() const;
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
    Builder &setPosition(const Position);
    Builder &setExtent(const Extent);
    Builder &setAlpha(const float);
    Builder &setCaption(const std::string_view);

    [[nodiscard]] Window build() const;

  private:
    const Window *m_parent{nullptr};
    Position m_position{};
    Extent m_extent{};
    float m_alpha{1.0f};
    std::string m_caption;
  };

private:
  Window(const Window *parent, const Position, const Extent, const float alpha,
         const std::string_view caption);

#if WIN32
  static LRESULT CALLBACK _wndProcRouter(HWND, UINT uMsg, WPARAM, LPARAM);
  LRESULT CALLBACK _wndProc(HWND, UINT uMsg, WPARAM, LPARAM);
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
  glm::ivec2 m_frame{0};
  Position m_clientOrigin{};
  Extent m_clientExtent{};

  State m_state{State::Windowed};
  bool m_focused{true};
#endif

  std::string m_caption;
  float m_alpha{0.0f}; // Opacity [0 = transparent, 1 = opaque].
};

[[nodiscard]] float getAspectRatio(const Window &);

// Moves a given window to the center of a primary monitor.
void center(Window &);
[[nodiscard]] glm::uvec2 getCenter(const Window &);

uint32_t pollEvents();

} // namespace os
