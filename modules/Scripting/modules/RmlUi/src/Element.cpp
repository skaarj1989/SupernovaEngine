#include "Element.hpp"
#include "RmlUi/Core/Context.h"
#include "RmlUi/Core/StyleSheet.h"
#include "RmlUi/Core/ElementDocument.h"
#include "RmlUi/Core/EventListener.h"
#include "EventListener.hpp"
#include "glm2Rml.hpp"
#include "Rml2glm.hpp"

#include "Sol2HelperMacros.hpp"

using namespace Rml;

void registerElement(sol::table &lua) {
  // clang-format off
  DEFINE_USERTYPE(Element,
    sol::call_constructor,
    sol::constructors<Element(const String &)>(),

    "setClass", &Element::SetClass,
    "isClassSet", &Element::IsClassSet,
    "setClassNames", &Element::SetClassNames,
    "getClassNames", &Element::GetClassNames,

    "getStyleSheet", &Element::GetStyleSheet,

    "getAddress", sol::overload(
      [](Element &self) { return self.GetAddress(); },
      [](Element &self, bool includePseudoClasses) {
        return self.GetAddress(includePseudoClasses);
      },
      &Element::GetAddress
    ),

    "setOffset", sol::overload(
      [](Element &self, const glm::vec2 offset, Element *parent) {
        self.SetOffset(to_Rml(offset), parent);
      },
      [](Element &self, const glm::vec2 offset, Element *parent, bool fixed) {
        self.SetOffset(to_Rml(offset), parent, fixed);
      }
    ),
    "getRelativeOffset", sol::overload(
      [](Element &self) {
        return to_glm(self.GetRelativeOffset());
      },
      [](Element &self, Box::Area area) {
        return to_glm(self.GetRelativeOffset(area));
      }
    ),
    "getAbsoluteOffset", sol::overload(
      [](Element &self) {
        return to_glm(self.GetAbsoluteOffset());
      },
      [](Element &self, Box::Area area) {
        return to_glm(self.GetAbsoluteOffset(area));
      }
    ),

    "setClientArea", &Element::SetClientArea,
    "getClientArea", &Element::GetClientArea,

    "setContentBox",
      [](Element &self, const glm::vec2 offset, const glm::vec2 box) {
        self.SetContentBox(to_Rml(offset), to_Rml(box));
      },
    "setBox", &Element::SetBox,
    "addBox", [](Element &self, const Box &box, const glm::vec2 offset) {
      self.AddBox(box, to_Rml(offset));
    },
    "getBox", sol::overload(
      sol::resolve<const Box &()>(&Element::GetBox),
      [](Element &self, int index) {
        Vector2f offset;
        const auto box = self.GetBox(index, offset);
        return std::tuple{box, to_glm(offset)};
      }
    ),
    "getNumBoxes", &Element::GetNumBoxes,

    "getBaseline", &Element::GetBaseline,
    "getIntrinsicDimensions", [](Element &self) {
      Vector2f dimensions_;
      float ratio;
      auto rv = self.GetIntrinsicDimensions(dimensions_, ratio);
      return std::tuple{to_glm(dimensions_), ratio};
    },

    "isPointWithinElement", [](Element &self, const glm::vec2 v) {
      return self.IsPointWithinElement(to_Rml(v));
    },

    "isVisible", sol::overload(
      [](const Element &self) { return self.IsVisible(); },
      &Element::IsVisible
    ),
    "getZIndex", &Element::GetZIndex,

    "getFontFaceHandle", &Element::GetFontFaceHandle,

    "setProperty", sol::overload(
      sol::resolve<bool(const String &, const String &)>(&Element::SetProperty),
      sol::resolve<bool(PropertyId, const Property &)>(&Element::SetProperty)
    ),
    "removeProperty", sol::overload(
      sol::resolve<void(const String &)>(&Element::RemoveProperty),
      sol::resolve<void(PropertyId)>(&Element::RemoveProperty)
    ),
    "getProperty", sol::overload(
      [](Element &self, const String &name) { return self.GetProperty(name); },
      [](Element &self, PropertyId id) { return self.GetProperty(id); }
    ),
    "getLocalProperty", sol::overload(
      sol::resolve<const Property *(const String &)>(&Element::GetLocalProperty),
      sol::resolve<const Property *(PropertyId)>(&Element::GetLocalProperty)
    ),
    
    "resolveNumericProperty", sol::overload(
      [](Element &self, const Property *property, float baseValue) {
        self.ResolveNumericProperty(property, baseValue);
      },
      [](Element &self, const String &name) {
        self.ResolveNumericProperty(name);
      }
    ),

    "getContainingBlock", [](Element &self) {
      return to_glm(self.GetContainingBlock());
    },
    "getPosition", &Element::GetPosition,
    "getFloat", &Element::GetFloat,
    "getDisplay", &Element::GetDisplay,
    "getLineHeight", &Element::GetLineHeight,

    "project", [](const Element &self, glm::vec2 &point) {
      auto point_ = to_Rml(point);
      const auto rv = self.Project(point_);
      point = to_glm(point_);
      return rv;
    },

    "animate", sol::overload(
      [](Element &self, const String &name, const Property &targetValue,
         float duration) { return self.Animate(name, targetValue, duration); },
      [](Element &self, const String &name, const Property &targetValue,
         float duration, Tween tween) {
        return self.Animate(name, targetValue, duration, tween);
      },
      [](Element &self, const String &name, const Property &targetValue,
         float duration, Tween tween, int numIterations) {
        return self.Animate(name, targetValue, duration, tween, numIterations);
      },
      [](Element &self, const String &name, const Property &targetValue,
         float duration, Tween tween, int numIterations,
         bool alternateDirection) {
        return self.Animate(name, targetValue, duration, tween, numIterations,
                            alternateDirection);
      },
      [](Element &self, const String &name, const Property &targetValue,
         float duration, Tween tween, int numIterations,
         bool alternateDirection, float delay) {
        return self.Animate(name, targetValue, duration, tween, numIterations,
                            alternateDirection, delay);
      },
      &Element::Animate
    ),
 
    "addAnimationKey", sol::overload(
      [](Element &self, const String &name, const Property &value, float duration) {
        return self.AddAnimationKey(name, value, duration);
      },
      &Element::AddAnimationKey
    ),

    "setPseudoClass", &Element::SetPseudoClass,
    "isPseudoClassSet", &Element::IsPseudoClassSet,
    "arePseudoClassesSet", &Element::ArePseudoClassesSet,
    "getActivePseudoClasses", &Element::GetActivePseudoClasses,

    "hasAttribute", &Element::HasAttribute,
    "removeAttribute", &Element::RemoveAttribute,

    "getNumAttributes", &Element::GetNumAttributes,
    
    "getFocusLeafNode", &Element::GetFocusLeafNode,

    "getContext", &Element::GetContext,
    
    "getTagName", &Element::GetTagName,
    "getId", &Element::GetId,
    "setId", &Element::SetId,

    "getAbsoluteLeft", &Element::GetAbsoluteLeft,
    "getAbsoluteTop", &Element::GetAbsoluteTop,
    
    "getClientLeft", &Element::GetClientLeft,
    "getClientTop", &Element::GetClientTop,
    "getClientWidth", &Element::GetClientWidth,
    "getClientHeight", &Element::GetClientHeight,

    "getOffsetParent", &Element::GetOffsetParent,
    "getOffsetLeft", &Element::GetOffsetLeft,
    "getOffsetTop", &Element::GetOffsetTop,
    "getOffsetWidth", &Element::GetOffsetWidth,
    "getOffsetHeight", &Element::GetOffsetHeight,
    
    "getScrollLeft", &Element::GetScrollLeft,
    "setScrollLeft", &Element::SetScrollLeft,
    "getScrollTop", &Element::GetScrollTop,
    "setScrollTop", &Element::SetScrollTop,
    "getScrollWidth", &Element::GetScrollWidth,
    "getScrollHeight", &Element::GetScrollHeight,
    
    "getOwnerDocument", &Element::GetOwnerDocument,
    
    "getParentNode", &Element::GetParentNode,
    "closest", &Element::Closest,

    "getNextSibling", &Element::GetNextSibling,
    "getPreviousSibling", &Element::GetPreviousSibling,

    "getFirstChild", &Element::GetFirstChild,
    "getLastChild", &Element::GetLastChild,
    "getChild", &Element::GetChild,
    "getNumChildren", sol::overload(
      [](Element &self) { return self.GetNumChildren(); },
      &Element::GetNumChildren
    ),

    "getInnerRML", sol::resolve<String() const>(&Element::GetInnerRML),
    "setInnerRML", &Element::SetInnerRML,

    "focus", &Element::Focus,
    "blur", &Element::Blur,
    "click", &Element::Click,

    "addEventListener", sol::overload(
      [](Element &self, const String &event, sol::table t) {
        return addEventListener(self, event, t);
      },
      [](Element &self, const String &event, sol::table t, bool inCapturePhase) {
        return addEventListener(self, event, t, inCapturePhase);
      },
      [](Element &self, EventId id, sol::table t) {
        return addEventListener(self, id, t);
      },
      [](Element &self, EventId id, sol::table t, bool inCapturePhase) {
        return addEventListener(self, id, t, inCapturePhase);
      }
    ),
    "removeEventListener", sol::overload(
      [](Element &self, EventId id, ScriptEventListener *listener) {
        self.RemoveEventListener(id, listener);
      },
      [](Element &self, EventId id, ScriptEventListener *listener,
         bool inCapturePahse) {
        self.RemoveEventListener(id, listener, inCapturePahse);
      }
    ),

    "scrollIntoView", sol::overload(
      [](Element &self) { self.ScrollIntoView(); },
      sol::resolve<void(bool)>(&Element::ScrollIntoView)
    ), 
    "scrollTo", sol::overload(
      [](Element &self, const glm::vec2 offset) {
        self.ScrollTo(to_Rml(offset));
      },  
      [](Element &self, const glm::vec2 offset, ScrollBehavior behavior) {
        self.ScrollTo(to_Rml(offset), behavior);
      }
    ),
    
    "hasChildNodes", &Element::HasChildNodes,

    "getElementById", &Element::GetElementById,
    "getElementsByTagName", &Element::GetElementsByTagName,
    "getElementsByClassName", &Element::GetElementsByClassName,
    "querySelector", &Element::QuerySelector,
    "querySelectorAll", &Element::QuerySelectorAll,
    
    sol::meta_function::to_string, [](const Element &self) {
      return std::format("Element(id = '{}')", self.GetId());
    }
  );
  // clang-format on
}
