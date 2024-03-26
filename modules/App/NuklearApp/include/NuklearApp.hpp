#include "BaseApp.hpp"
#include "NuklearRenderer.hpp"

class NuklearApp : public BaseApp {
public:
  NuklearApp(std::span<char *> args, const Config &);
  ~NuklearApp() override;

  [[nodiscard]] nk_context *getNuklearContext() const;

  void drawGui(rhi::CommandBuffer &, const rhi::RenderTargetView,
               const std::optional<glm::vec4> clearColor);

protected:
  void _onPreUpdate(const fsec dt) override;

  void _onInput(const os::InputEvent &) override;
  void _notify(std::span<bool> keysDown) const;

  void _onPostUpdate(const fsec dt) override;

  void _onRender(rhi::CommandBuffer &, const rhi::RenderTargetView,
                 const fsec dt) override;

private:
  std::unique_ptr<nk_context> m_nuklearContext;
  nk_font_atlas m_fontAtlas;

  std::unique_ptr<NuklearRenderer> m_uiRenderer;
  std::vector<NuklearRenderer::FrameResources> m_uiResources;
};
