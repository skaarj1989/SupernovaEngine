#include "ImGuiApp.hpp"
#include "VisitorHelper.hpp"
#include "os/FileSystem.hpp"
#include "ImGuiPlatformInterface.hpp"
#include "ImGuiRenderInterface.hpp"
#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "glm/ext/vector_float3.hpp"

namespace {

void setupIcons(ImFontAtlas &fonts, const std::filesystem::path &dir,
                std::initializer_list<std::string_view> filenames) {
  assert(filenames.size() > 0);

  fonts.AddFontDefault();

  ImFontConfig iconsConfig{};
  iconsConfig.MergeMode = true;
  iconsConfig.PixelSnapH = true;
  iconsConfig.GlyphMinAdvanceX = 13.0f;

  constexpr auto kFontSize = 10.0f;
  constexpr auto kGlyphRanges =
    std::array<ImWchar, 3>{ICON_MIN_FA, ICON_MAX_FA, 0};

  for (const auto fontName : filenames) {
    if (auto buffer = os::FileSystem::readBuffer(dir / fontName); buffer) {
      fonts.AddFontFromMemoryTTF(buffer->data.release(),
                                 static_cast<int32_t>(buffer->size), kFontSize,
                                 &iconsConfig, kGlyphRanges.data());
    }
  }
  fonts.Build();
}

} // namespace

ImGuiApp::ImGuiApp(std::span<char *> args, const Config &config,
                   const std::filesystem::path &iconsDir)
    : BaseApp{args, config} {
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();
  auto &io = ImGui::GetIO();

  io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;

#ifdef IMGUI_HAS_DOCK
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // = true, fixes ugly jittering (still present in ImGui 1.89.7).
  io.ConfigDockingTransparentPayload = true;
  io.ConfigDockingWithShift = true;
#endif
#ifdef IMGUI_HAS_VIEWPORT
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

  if (!iconsDir.empty()) {
    setupIcons(*io.Fonts, iconsDir,
               {FONT_ICON_FILE_NAME_FAR, FONT_ICON_FILE_NAME_FAS});
  }
  m_uiRenderer = std::make_unique<ImGuiRenderer>(getRenderDevice());
  m_uiResources = m_uiRenderer->createResources(config.numFramesInFlight);

  setupPlatformInterface(getWindow());
  setupRenderInterface(*m_uiRenderer);

#ifdef IMGUI_HAS_VIEWPORT
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    auto &style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
#endif
}
ImGuiApp::~ImGuiApp() {
  getRenderDevice().waitIdle();

#ifdef IMGUI_HAS_VIEWPORT
  ImGui::DestroyPlatformWindows();
#endif

  m_uiResources.clear();
  m_uiRenderer.reset();
  ImGui::GetIO().BackendRendererUserData = nullptr;

  ImGui::DestroyContext();
}

void ImGuiApp::drawGui(rhi::CommandBuffer &cb, const rhi::RenderTargetView rtv,
                       std::optional<glm::vec4> clearColor) {
  RHI_GPU_ZONE(cb, "ImGui::Main");
  auto &[frameIndex, target] = rtv;
  rhi::prepareForAttachment(cb, target, false);
  cb.beginRendering({
    .area = {.extent = target.getExtent()},
    .colorAttachments = {{
      .target = &target,
      .clearValue = std::move(clearColor),
    }},
  });
  m_uiRenderer->draw(cb, target.getPixelFormat(), m_uiResources[frameIndex],
                     ImGui::GetDrawData());
  cb.endRendering();
}

//
// (protected):
//

void ImGuiApp::_onResizeWindow(const os::ResizeWindowEvent &evt) {
  BaseApp::_onResizeWindow(evt);
  ImGui::GetIO().DisplaySize = glm::vec2{evt.size};
}

void ImGuiApp::_onInput(const os::InputEvent &evt) {
  BaseApp::_onInput(evt);
  std::visit(
    Overload{
      [this](const os::MouseMoveEvent &evt_) { _onMouseMove(evt_); },
      [this](const os::MouseButtonEvent &evt_) { _onMouseButton(evt_); },
      [this](const os::MouseWheelEvent &evt_) { _onMouseWheel(evt_); },
      [this](const os::KeyboardEvent &evt_) { _onKeyboard(evt_); },
      [this](const os::InputCharacterEvent &evt_) { _onInputCharacter(evt_); },
    },
    evt);
}

void ImGuiApp::_onMouseMove([[maybe_unused]] const os::MouseMoveEvent &evt) {
#if !defined(IMGUI_HAS_VIEWPORT)
  if (getWindow().hasFocus()) {
    auto &io = ImGui::GetIO();
    io.AddMousePosEvent(float(evt.position.x), float(evt.position.y));
  }
#endif
}
void ImGuiApp::_onMouseButton(const os::MouseButtonEvent &evt) {
  auto &io = ImGui::GetIO();
  io.AddMouseButtonEvent(int32_t(evt.button),
                         evt.state == os::MouseButtonState::Pressed);
}
void ImGuiApp::_onMouseWheel(const os::MouseWheelEvent &evt) {
  auto x = evt.step;
  decltype(x) y = 0;
  if (evt.wheel == os::MouseWheel::Vertical) std::swap(x, y);
  ImGui::GetIO().AddMouseWheelEvent(x, y);
}
void ImGuiApp::_onKeyboard(const os::KeyboardEvent &evt) {
  auto &io = ImGui::GetIO();
  const auto pressed = evt.state == os::KeyState::Down;

  if (const auto key = remapKeyCode(evt.keyCode); key) {
    io.AddKeyEvent(*key, pressed);
  }
  if (const auto modifier = getKeyModifier(evt.keyCode); modifier) {
    io.AddKeyEvent(*modifier, pressed);
  }
}
void ImGuiApp::_onInputCharacter(const os::InputCharacterEvent &evt) {
  ImGui::GetIO().AddInputCharacterUTF16(evt.c);
}

void ImGuiApp::_onPostUpdate(fsec dt) {
  auto &io = ImGui::GetIO();
  io.DeltaTime = dt.count();

#ifdef IMGUI_HAS_VIEWPORT
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    const auto p = glm::vec2{getInputSystem().getMousePosition()};
    io.AddMousePosEvent(p.x, p.y);
  }
#endif
}
void ImGuiApp::_onRender(rhi::CommandBuffer &cb,
                         const rhi::RenderTargetView rtv, const fsec) {
  constexpr auto kClearColor = glm::vec3{0.0f};
  drawGui(cb, rtv, glm::vec4{kClearColor, 1.0f});
}
void ImGuiApp::_onPostRender() {
#ifdef IMGUI_HAS_VIEWPORT
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
#endif
}
