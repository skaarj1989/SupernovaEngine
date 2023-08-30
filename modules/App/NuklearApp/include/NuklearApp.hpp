#include "BaseApp.hpp"
#include "NuklearRenderer.hpp"

class NuklearApp : public BaseApp {
public:
  NuklearApp(std::span<char *> args, const Config &);
  ~NuklearApp() override;

  [[nodiscard]] nk_context *getNuklearContext() const;

  void drawGui(rhi::CommandBuffer &, rhi::RenderTargetView,
               std::optional<glm::vec4> clearColor);

protected:
  void _onPreUpdate(fsec dt) override;

  void _onInput(const os::InputEvent &) override;
  void _notify(std::span<bool> keysDown) const;

  void _onPostUpdate(fsec dt) override;

  void _onRender(rhi::CommandBuffer &, rhi::RenderTargetView, fsec dt) override;

private:
  std::unique_ptr<nk_context> m_nuklearContext;
  nk_font_atlas m_fontAtlas;

  std::unique_ptr<NuklearRenderer> m_uiRenderer;
  std::vector<NuklearRenderer::FrameResources> m_uiResources;
};
