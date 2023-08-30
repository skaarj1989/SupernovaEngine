#pragma once

#include "RenderDoc.hpp"
#include "rhi/FrameController.hpp"
#include "os/InputSystem.hpp"
#include "spdlog/spdlog.h"

using fsec = std::chrono::duration<float>;

class BaseApp : public RenderDoc {
public:
  struct Config {
    std::string caption;
    uint32_t width{1280};
    uint32_t height{720};
    std::optional<rhi::Vendor> vendor;
    bool verticalSync{true};
  };
  static constexpr auto kFramesInFlight = 2;

  BaseApp(std::span<char *> args, const Config &);
  BaseApp(const BaseApp &) = delete;
  BaseApp(BaseApp &&) noexcept = delete;
  ~BaseApp() override = default;

  BaseApp &operator=(const BaseApp &) = delete;
  BaseApp &operator=(BaseApp &&) noexcept = delete;

  [[nodiscard]] os::Window &getWindow();
  [[nodiscard]] os::InputSystem &getInputSystem();
  [[nodiscard]] rhi::RenderDevice &getRenderDevice();
  [[nodiscard]] rhi::Swapchain &getSwapchain();

  void run();
  void close();

protected:
  void _setupWindowCallbacks();

  virtual void _onResizeWindow(const os::ResizeWindowEvent &);
  virtual void _onInput(const os::InputEvent &);

  virtual void _onPreUpdate(fsec) {}
  virtual void _onUpdate(fsec) {}
  virtual void _onPhysicsUpdate(fsec) {}
  virtual void _onPostUpdate(fsec) {}

  virtual void _onPreRender() {}
  virtual void _onRender(rhi::CommandBuffer &, rhi::RenderTargetView, fsec) {}

  virtual void _onPostRender() {}

private:
  using RenderDoc::_beginFrame;
  using RenderDoc::_endFrame;

private:
  bool m_quit{false};

  os::Window m_window;
  std::unique_ptr<rhi::RenderDevice> m_renderDevice;
  rhi::Swapchain m_swapchain;
  rhi::FrameController m_frameController;

  os::InputSystem m_inputSystem;
};

#define CONFIG_MAIN(AppClass)                                                  \
  int main(int argc, char *argv[]) {                                           \
    try {                                                                      \
      AppClass app{std::span{argv, std::size_t(argc)}};                        \
      app.run();                                                               \
    } catch (const std::exception &e) {                                        \
      SPDLOG_CRITICAL(e.what());                                               \
      return -1;                                                               \
    }                                                                          \
    return 0;                                                                  \
  }
