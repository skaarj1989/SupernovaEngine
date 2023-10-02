#pragma once

#include "sol/table.hpp"
#include "RmlUi/Core/EventListener.h"
#include "RmlUi/Core/Element.h"

class ScriptEventListener final : public Rml::EventListener {
public:
  explicit ScriptEventListener(const sol::table &t)
      : m_self{t}, m_processEvent{m_self["processEvent"]} {}

  void ProcessEvent(Rml::Event &evt) override {
    if (m_processEvent) m_processEvent(evt);
  }

  void OnAttach(Rml::Element *e) override {
    if (sol::protected_function f = m_self["onAttach"]; f) f(e);
  }
  void OnDetach(Rml::Element *e) override {
    if (sol::protected_function f = m_self["onDetach"]; f) f(e);

    // https://isocpp.org/wiki/faq/freestore-mgmt#delete-this
    delete this;
  }

private:
  sol::table m_self;
  sol::protected_function m_processEvent;
};

template <class T, class U>
auto addEventListener(T &self, U &&event, sol::table t,
                      bool inCapturePhase = false) {
  auto *listener = new ScriptEventListener{t};
  self.AddEventListener(std::forward<U>(event), listener, inCapturePhase);
  return listener;
}
