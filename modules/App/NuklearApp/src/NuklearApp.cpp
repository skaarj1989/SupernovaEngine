#include "NuklearApp.hpp"
#include "VisitorHelper.hpp"
#include "glm/ext/vector_float3.hpp"

NuklearApp::NuklearApp(std::span<char *> args, const Config &config)
    : BaseApp{args, config} {
  m_nuklearContext = std::make_unique<nk_context>();
  auto *const ctx = getNuklearContext();
  nk_init_default(ctx, nullptr);

  nk_font_atlas_init_default(&m_fontAtlas);
  nk_font_atlas_begin(&m_fontAtlas);

  m_uiRenderer =
    std::make_unique<NuklearRenderer>(getRenderDevice(), *ctx, m_fontAtlas);
  m_uiResources = m_uiRenderer->createResources(kFramesInFlight);
}
NuklearApp::~NuklearApp() {
  getRenderDevice().waitIdle();

  m_uiRenderer.reset();
  m_uiResources.clear();

  nk_font_atlas_clear(&m_fontAtlas);
  nk_free(getNuklearContext());
}

nk_context *NuklearApp::getNuklearContext() const {
  return m_nuklearContext.get();
}

void NuklearApp::drawGui(rhi::CommandBuffer &cb, rhi::RenderTargetView rtv,
                         std::optional<glm::vec4> clearColor) {
  auto &[frameIndex, target] = rtv;
  const auto extent = target.getExtent();
  rhi::prepareForAttachment(cb, rtv.texture, false);
  cb.beginRendering({
    .area = {.extent = extent},
    .colorAttachments = {{
      .target = &target,
      .clearValue = std::move(clearColor),
    }},
  });
  m_uiRenderer->draw(cb, m_uiResources[frameIndex], extent);
  cb.endRendering();
}

void NuklearApp::_onPreUpdate(fsec) {
  auto *ctx = getNuklearContext();
  nk_clear(ctx);
  nk_input_begin(ctx);
}

void NuklearApp::_onInput(const os::InputEvent &evt) {
  BaseApp::_onInput(evt);
  std::visit(
    Overload{
      [this](const os::MouseMoveEvent &evt_) {
        nk_input_motion(getNuklearContext(), evt_.position.x, evt_.position.y);
      },
      [this](const os::MouseButtonEvent &evt_) {
        nk_input_button(getNuklearContext(), nk_buttons(evt_.button),
                        evt_.position.x, evt_.position.y,
                        evt_.state == os::MouseButtonState::Pressed);
      },
      [this](const os::MouseWheelEvent &evt_) {
        struct nk_vec2 val {};
        *(evt_.wheel == os::MouseWheel::Horizontal ? &val.x : &val.y) +=
          evt_.step;
        nk_input_scroll(getNuklearContext(), val);
      },
      [this](const os::KeyboardEvent &evt_) {
        std::array<bool, 256> keysDown{};
        keysDown[int32_t(evt_.keyCode)] = evt_.state == os::KeyState::Down;
        _notify(keysDown);
      },
      [this](const os::InputCharacterEvent &evt_) {
        nk_input_unicode(getNuklearContext(), evt_.c);
      },
    },
    evt);
}
void NuklearApp::_notify(std::span<bool> keysDown) const {
  auto *const ctx = getNuklearContext();

#define GET_KEY(Key) keysDown[int32_t(os::KeyCode::Key)]

  nk_input_key(ctx, NK_KEY_DEL, GET_KEY(Delete));
  nk_input_key(ctx, NK_KEY_ENTER, GET_KEY(Return));
  nk_input_key(ctx, NK_KEY_TAB, GET_KEY(Tab));
  nk_input_key(ctx, NK_KEY_BACKSPACE, GET_KEY(Backspace));
  nk_input_key(ctx, NK_KEY_UP, GET_KEY(Up));
  nk_input_key(ctx, NK_KEY_DOWN, GET_KEY(Down));
  nk_input_key(ctx, NK_KEY_TEXT_START, GET_KEY(Home));
  nk_input_key(ctx, NK_KEY_TEXT_END, GET_KEY(End));
  nk_input_key(ctx, NK_KEY_SCROLL_START, GET_KEY(Home));
  nk_input_key(ctx, NK_KEY_SCROLL_END, GET_KEY(End));
  nk_input_key(ctx, NK_KEY_SCROLL_DOWN, GET_KEY(Next));
  nk_input_key(ctx, NK_KEY_SCROLL_UP, GET_KEY(Prior));
  nk_input_key(ctx, NK_KEY_SHIFT, GET_KEY(LShift) || GET_KEY(RShift));

  if (GET_KEY(LControl) || GET_KEY(RControl)) {
    nk_input_key(ctx, NK_KEY_COPY, keysDown['C']);
    nk_input_key(ctx, NK_KEY_PASTE, keysDown['V']);
    nk_input_key(ctx, NK_KEY_CUT, keysDown['X']);
    nk_input_key(ctx, NK_KEY_TEXT_UNDO, keysDown['Z']);
    nk_input_key(ctx, NK_KEY_TEXT_REDO, keysDown['R']);
    nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, GET_KEY(Left));
    nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, GET_KEY(Right));
    nk_input_key(ctx, NK_KEY_TEXT_LINE_START, keysDown['B']);
    nk_input_key(ctx, NK_KEY_TEXT_LINE_END, keysDown['E']);
  } else {
    nk_input_key(ctx, NK_KEY_LEFT, GET_KEY(Left));
    nk_input_key(ctx, NK_KEY_RIGHT, GET_KEY(Right));
    nk_input_key(ctx, NK_KEY_COPY, 0);
    nk_input_key(ctx, NK_KEY_PASTE, 0);
    nk_input_key(ctx, NK_KEY_CUT, 0);
    nk_input_key(ctx, NK_KEY_SHIFT, 0);
  }

#undef GET_KEY
}

void NuklearApp::_onPostUpdate(fsec) { nk_input_end(getNuklearContext()); }

void NuklearApp::_onRender(rhi::CommandBuffer &cb, rhi::RenderTargetView rtv,
                           fsec) {
  constexpr auto kClearColor = glm::vec3{0.0f};
  drawGui(cb, rtv, glm::vec4{kClearColor, 1.0f});
}
