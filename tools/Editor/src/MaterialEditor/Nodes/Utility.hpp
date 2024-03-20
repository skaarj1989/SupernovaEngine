#pragma once

#include "MaterialEditor/Nodes/NodeBase.hpp"
#include "MaterialEditor/ShaderGraph.hpp"
#include "MaterialEditor/TransientVariant.hpp"

[[nodiscard]] NodeBase *createNode(ShaderGraph &, std::optional<VertexID> hint,
                                   const TransientVariant &);
