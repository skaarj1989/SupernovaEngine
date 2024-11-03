#include "BaseApp.hpp"
#include "TypeTraits.hpp"
#include "os/Monitor.hpp"

#define LOG_TO_CONSOLE 1

#if LOG_TO_CONSOLE
#  include "spdlog/sinks/dup_filter_sink.h"
#  include "spdlog/sinks/stdout_color_sinks.h"
#else
#  include "spdlog/sinks/basic_file_sink.h"
#endif

#include "tracy/Tracy.hpp"

#if WIN32 && _DEBUG
#  include <crtdbg.h> // _CrtSet*
#endif

namespace {

using namespace std::chrono_literals;

void setupLogger(const std::string &name) {
#if LOG_TO_CONSOLE
  auto duplicateFilter =
    std::make_unique<spdlog::sinks::dup_filter_sink_st>(3s);
  duplicateFilter->add_sink(
    std::make_unique<spdlog::sinks::stdout_color_sink_mt>());

  auto logger =
    std::make_shared<spdlog::logger>(name, std::move(duplicateFilter));
#else
  auto logger = spdlog::basic_logger_st(name, "log.txt");
#endif
  spdlog::set_default_logger(std::move(logger));
}

class FPSMonitor final {
public:
  explicit FPSMonitor(os::Window &window)
      : m_target{window}, m_originalCaption{window.getCaption()} {}

  void update(const fsec dt) {
    ++m_numFrames;
    m_time += dt;

    if (m_time >= 1s) {
      m_target.setCaption(
        std::format("{} | FPS = {}", m_originalCaption, m_numFrames));

      m_time = 0s;
      m_numFrames = 0;
    }
  }

private:
  os::Window &m_target;
  const std::string m_originalCaption;

  uint32_t m_numFrames{0};
  fsec m_time{0s};
};

} // namespace

//
// ExampleApp class:
//

BaseApp::BaseApp(std::span<char *>, const Config &config) : RenderDoc{} {
#if WIN32 && _DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
  setupLogger(config.caption);

  m_renderDevice = std::make_unique<rhi::RenderDevice>(
    config.vendor ? rhi::selectVendor(*config.vendor)
                  : rhi::defaultDeviceSelector);
  m_window = os::Window::Builder{}
               .setCaption(std::format("{} ({})", config.caption,
                                       m_renderDevice->getName()))
               .setExtent({config.width, config.height})
               .build();
  os::center(m_window);

  const auto vSync = config.verticalSync ? rhi::VerticalSync::Enabled
                                         : rhi::VerticalSync::Disabled;

  m_swapchain = m_renderDevice->createSwapchain(
    m_window, rhi::Swapchain::Format::Linear, vSync);
  m_frameController = rhi::FrameController{*m_renderDevice, m_swapchain,
                                           config.numFramesInFlight};

  _setupWindowCallbacks();
}

os::Window &BaseApp::getWindow() { return m_window; }
os::InputSystem &BaseApp::getInputSystem() { return m_inputSystem; }
rhi::RenderDevice &BaseApp::getRenderDevice() { return *m_renderDevice; }
rhi::Swapchain &BaseApp::getSwapchain() { return m_swapchain; }

void BaseApp::run() {
  FPSMonitor fpsMonitor{m_window};

  const fsec targetFrameTime{1.0 / 60.0f};
  fsec deltaTime{targetFrameTime};
  fsec accumulator{0};

  // https://gafferongames.com/post/fix_your_timestep/
  // http://gameprogrammingpatterns.com/game-loop.html
  // https://dewitters.com/dewitters-gameloop/
  // http://higherorderfun.com/blog/2010/08/17/understanding-the-game-main-loop/

  m_window.show();
  while (true) {
    using clock = std::chrono::high_resolution_clock;
    const auto beginTicks = clock::now();

    {
      ZoneScopedN("[App]PreUpdate");
      _onPreUpdate(deltaTime);
    }
    {
      ZoneScopedN("[App]PollEvents");
      os::pollEvents();
    }
    if (m_quit) break;

    m_inputSystem.update();
    {
      ZoneScopedN("[App]Update");
      _onUpdate(deltaTime);
    }
    {
      ZoneScopedN("[App]PhysicsUpdate");
      accumulator +=
        (deltaTime < targetFrameTime ? deltaTime : targetFrameTime);
      while (accumulator >= targetFrameTime) {
        _onPhysicsUpdate(targetFrameTime);
        accumulator -= targetFrameTime;
      }
    }
    {
      ZoneScopedN("[App]PostUpdate");
      _onPostUpdate(deltaTime);
    }

    if (m_swapchain) {
      {
        ZoneScopedN("[App]PreRender");
        _onPreRender();
      }
      {
        ZoneScopedN("[App]Render");
        RenderDoc::_beginFrame();
        auto &cb = m_frameController.beginFrame();
        m_renderDevice->stepGarbage(m_frameController.size());
        _onRender(cb, m_frameController.getCurrentTarget(), deltaTime);
        m_frameController.endFrame();
        RenderDoc::_endFrame();
      }
      {
        ZoneScopedN("[App]PostRender");
        _onPostRender();
      }
      {
        ZoneScopedN("[App]Present");
        m_frameController.present();
      }
    } // else -> Window is minimized.
    FrameMark;

    deltaTime = clock::now() - beginTicks;
    if (deltaTime > 1s) deltaTime = targetFrameTime;

    fpsMonitor.update(deltaTime);
  }
}
void BaseApp::close() { m_quit = true; }

//
// (private):
//

void BaseApp::_setupWindowCallbacks() {
  m_window.on<os::CloseWindowEvent>(
    [this](const auto, const auto &) { close(); });
  m_window.on<os::ResizeWindowEvent>(
    [this](const auto &evt, const auto &) { _onResizeWindow(evt); });

  const auto inputCallback = [this](const auto &evt, const auto &) {
    _onInput(evt);
  };

  m_window.on<os::MouseMoveEvent>(inputCallback);
  m_window.on<os::MouseButtonEvent>(inputCallback);
  m_window.on<os::MouseWheelEvent>(inputCallback);

  m_window.on<os::KeyboardEvent>(inputCallback);
  m_window.on<os::InputCharacterEvent>(inputCallback);
}

void BaseApp::_onResizeWindow(const os::ResizeWindowEvent &) {
  m_swapchain.recreate();
}
void BaseApp::_onInput(const os::InputEvent &evt) {
  std::visit(
    [this](const auto &evt_) {
      using T = std::decay_t<decltype(evt_)>;

      if constexpr (is_any_v<T, os::MouseButtonEvent, os::KeyboardEvent>) {
        m_inputSystem.notify(evt_);
      }
    },
    evt);
}
