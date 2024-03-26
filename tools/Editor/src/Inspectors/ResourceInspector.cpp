#include "Inspectors/ResourceInspector.hpp"
#include "Resource.hpp"
#include "IconsFontAwesome6.h"
#include "imgui.h"

void print(const Resource *resource) {
  ImGui::Text(ICON_FA_FILE " Resource: %s",
              resource ? toString(*resource).c_str() : "(none)");
}
