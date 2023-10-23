#include "RmlUiApp.hpp"
#include "RmlUi/Core.h"
#include "RmlUi/Debugger.h"

class DemoWindow {
public:
  DemoWindow(const Rml::String &title, Rml::Context &ctx) {
    using namespace Rml;

    m_document = ctx.LoadDocument("./demo-assets/ui/animation.rml");
    if (!m_document) return;

    m_document->GetElementById("title")->SetInnerRML(title);

    // Button fun
    {
      auto *el = m_document->GetElementById("start_game");
      const auto p1 = Transform::MakeProperty({
        Transforms::Rotate2D{10.f},
        Transforms::TranslateX{100.f},
      });
      const auto p2 = Transform::MakeProperty({Transforms::Scale2D{3.0f}});
      el->Animate("transform", p1, 1.8f, Tween{Tween::Elastic, Tween::InOut},
                  -1, true);
      el->AddAnimationKey("transform", p2, 1.3f,
                          Tween{Tween::Elastic, Tween::InOut});
    }
    {
      auto *el = m_document->GetElementById("high_scores");
      el->Animate("margin-left", Property{0.0f, Unit::PX}, 0.3f,
                  Tween{Tween::Sine, Tween::In}, 10, true, 1.0f);
      el->AddAnimationKey("margin-left", Property{100.f, Unit::PX}, 3.0f,
                          Tween{Tween::Circular, Tween::Out});
    }
    {
      auto *el = m_document->GetElementById("options");
      el->Animate("image-color",
                  Property{Colourb{128, 255, 255, 255}, Unit::COLOUR}, 0.3f,
                  Tween{}, -1, false);
      el->AddAnimationKey("image-color",
                          Property{Colourb{128, 128, 255, 255}, Unit::COLOUR},
                          0.3f);
      el->AddAnimationKey(
        "image-color", Property{Colourb{0, 128, 128, 255}, Unit::COLOUR}, 0.3f);
      el->AddAnimationKey(
        "image-color", Property{Colourb{64, 128, 255, 0}, Unit::COLOUR}, 0.9f);
      el->AddAnimationKey("image-color",
                          Property{Colourb{255, 255, 255, 255}, Unit::COLOUR},
                          0.3f);
    }
    {
      auto *el = m_document->GetElementById("exit");
      PropertyDictionary pd;
      StyleSheetSpecification::ParsePropertyDeclaration(
        pd, "transform", "translate(200px, 200px) rotate(1215deg)");
      el->Animate("transform", *pd.GetProperty(PropertyId::Transform), 3.0f,
                  Tween{Tween::Bounce, Tween::Out}, -1);
    }

    // Transform tests
    {
      auto el = m_document->GetElementById("generic");
      const auto p = Transform::MakeProperty({
        Transforms::TranslateY{50, Unit::PX},
        Transforms::Rotate3D{0, 0, 1, -90, Unit::DEG},
        Transforms::ScaleY{0.8f},
      });
      el->Animate("transform", p, 1.5f, Tween{Tween::Sine, Tween::InOut}, -1,
                  true);
    }
    {
      auto *el = m_document->GetElementById("combine");
      const auto p = Transform::MakeProperty({
        Transforms::Translate2D{50, 50, Unit::PX},
        Transforms::Rotate2D(1215),
      });
      el->Animate("transform", p, 8.0f, Tween{}, -1, true);
    }
    {
      auto *el = m_document->GetElementById("decomposition");
      const auto p = Transform::MakeProperty({
        Transforms::TranslateY{50, Unit::PX},
        Transforms::Rotate3D{0.8f, 0, 1, 110, Unit::DEG},
      });
      el->Animate("transform", p, 1.3f, Tween{Tween::Quadratic, Tween::InOut},
                  -1, true);
    }

    // Mixed units tests
    {
      auto *el = m_document->GetElementById("abs_rel");
      el->Animate("margin-left", Property{50.0f, Unit::PERCENT}, 1.5f, Tween{},
                  -1, true);
    }
    {
      auto *el = m_document->GetElementById("abs_rel_transform");
      auto p = Transform::MakeProperty({Transforms::TranslateX{0, Unit::PX}});
      el->Animate("transform", p, 1.5f, Tween{}, -1, true);
    }
    {
      auto *el = m_document->GetElementById("animation_event");
      el->Animate("top", Property{Math::RandomReal(250.0f), Unit::PX}, 1.5f,
                  Tween{Tween::Cubic, Tween::InOut});
      el->Animate("left", Property{Math::RandomReal(250.0f), Unit::PX}, 1.5f,
                  Tween{Tween::Cubic, Tween::InOut});
    }

    m_document->Show();
  }
  ~DemoWindow() {
    if (m_document) m_document->Close();
  }

  void update(float t) {
    if (!m_document) return;

    if (t - m_prevFade >= 1.4f) {
      auto *el = m_document->GetElementById("help");
      if (el->IsClassSet("fadeout")) {
        el->SetClass("fadeout", false);
        el->SetClass("fadein", true);
      } else if (el->IsClassSet("fadein")) {
        el->SetClass("fadein", false);
        el->SetClass("textalign", true);
      } else {
        el->SetClass("textalign", false);
        el->SetClass("fadeout", true);
      }

      m_prevFade = t;
    }
  }

  [[nodiscard]] auto *getDocument() { return m_document; }

private:
  Rml::ElementDocument *m_document{nullptr};
  float m_prevFade{0.0f};
};

class DemoApp final : public RmlUiApp {
public:
  explicit DemoApp(std::span<char *> args)
      : RmlUiApp{args, {.caption = "RmlUi Demo"}} {
    // clang-format off
    const auto fonts = {
      "LatoLatin-Regular.ttf",
      "LatoLatin-Italic.ttf",
      "LatoLatin-Bold.ttf",
      "LatoLatin-BoldItalic.ttf",
      "NotoEmoji-Regular.ttf",
    };
    // clang-format on
    for (const auto filename : fonts) {
      Rml::LoadFontFace(std::format("./demo-assets/ui/{}", filename));
    }
    m_demoWindow =
      Rml::MakeUnique<DemoWindow>("Animation sample", getUiContext());

#ifdef _DEBUG
    Rml::Debugger::SetVisible(true);
#endif
  }

private:
  void _onUpdate(fsec dt) override {
    m_demoWindow->update(dt.count());
    RmlUiApp::_onUpdate(dt);
  }

private:
  Rml::UniquePtr<DemoWindow> m_demoWindow;
};

CONFIG_MAIN(DemoApp);
