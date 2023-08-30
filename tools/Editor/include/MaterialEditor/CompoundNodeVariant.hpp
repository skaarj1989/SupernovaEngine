#pragma once

#include "Nodes/Custom.hpp"
#include "Nodes/Scripted.hpp"
#include "Nodes/Arithmetic.hpp"
#include "Nodes/Append.hpp"
#include "Nodes/VectorSplitter.hpp"
#include "Nodes/MatrixSplitter.hpp"
#include "Nodes/Swizzle.hpp"
#include "Nodes/MatrixTransform.hpp"
#include "Nodes/TextureSampling.hpp"
#include <variant>

// clang-format off

// NOTE: If you add a new node type then add appropriate entry to:
// NodeContextMenuEntries.cpp -> buildNodeMenuEntries()
using CompoundNodeVariant = std::variant<
  CustomNode,
  ScriptedNode,
  ArithmeticNode,
  AppendNode,
  VectorSplitterNode,
  MatrixSplitterNode,
  SwizzleNode,
  MatrixTransformNode,
  TextureSamplingNode
>;
// clang-format on

[[nodiscard]] const char *toString(const CompoundNodeVariant &);
