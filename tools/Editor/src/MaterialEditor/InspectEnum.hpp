#pragma once

#include "MaterialEditor/DataType.hpp"
#include <type_traits>

template <typename T>
  requires std::is_scoped_enum_v<T>
void inspectNode(int32_t id, std::optional<const char *> userLabel, const T e) {
  const auto kDefaultLabel = toString(e);
  const auto cstr = userLabel.value_or(kDefaultLabel);

  constexpr auto kMinWidth = 50.0f;
  const auto nodeWidth = glm::max(ImGui::CalcTextSize(cstr).x, kMinWidth);

  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted(cstr);
  ImNodes::EndNodeTitleBar();

  ImNodes::AddOutputAttribute(id, {
                                    .name = toString(getDataType(e)),
                                    .color = ImColor{86, 156, 214},
                                    .nodeWidth = nodeWidth,
                                  });
}
