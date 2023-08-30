#pragma once

#include "Nodes/VertexMaster.hpp"
#include "Nodes/SurfaceMaster.hpp"
#include "Nodes/PostProcessMaster.hpp"

#include <variant>

using MasterNodeVariant =
  std::variant<VertexMasterNode, SurfaceMasterNode, PostProcessMasterNode>;

[[nodiscard]] const char *toString(const MasterNodeVariant &);
