#include "MaterialEditor/Nodes/NodeCommon.hpp"
#include "MaterialEditor/ShaderGraph.hpp"

void CustomizableNode::remove(ShaderGraph &g) { removeVertices(g, inputs); }
