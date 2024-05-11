#pragma once

#include "Nodes/NodeBase.hpp"
#include "ShaderGraph.hpp"
#include "TransientVariant.hpp"

[[nodiscard]] NodeBase *createNode(ShaderGraph &, std::optional<VertexID> hint,
                                   const TransientVariant &);
