#pragma once

#include "MaterialEditor/Nodes/NodeCommon.hpp"
#include "renderer/MaterialProperty.hpp"

using PropertyValue = gfx::Property::Value;

[[nodiscard]] DataType getDataType(const PropertyValue &);

bool inspectNode(int32_t id, std::optional<const char *> userLabel,
                 PropertyValue &);

NodeResult evaluate(MaterialGenerationContext &, int32_t id,
                    std::optional<const char *> userLabel,
                    const PropertyValue &);
