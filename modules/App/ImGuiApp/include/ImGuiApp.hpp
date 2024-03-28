#include "BaseApp.hpp"
#include "ImGuiRenderer.hpp"

// HINT: Place your ImGui calls inside _onPreRender()

class ImGuiApp : public BaseApp {
public:
  ImGuiApp(std::span<char *> args, const Config &,
           const std::filesystem::path &iconsDir = "./fonts");
  ~ImGuiApp() override;

  void drawGui(rhi::CommandBuffer &, const rhi::RenderTargetView,
               std::optional<glm::vec4> clearColor = std::nullopt);

protected:
  void _onResizeWindow(const os::ResizeWindowEvent &) override;

  void _onInput(const os::InputEvent &) override;

  void _onMouseMove(const os::MouseMoveEvent &);
  void _onMouseButton(const os::MouseButtonEvent &);
  void _onMouseWheel(const os::MouseWheelEvent &);

  void _onKeyboard(const os::KeyboardEvent &);
  void _onInputCharacter(const os::InputCharacterEvent &);

  void _onPostUpdate(const fsec dt) override;

  void _onRender(rhi::CommandBuffer &, const rhi::RenderTargetView,
                 const fsec dt) override;
  void _onPostRender() override;

private:
  std::unique_ptr<ImGuiRenderer> m_uiRenderer;
  std::vector<ImGuiRenderer::FrameResources> m_uiResources;
};
