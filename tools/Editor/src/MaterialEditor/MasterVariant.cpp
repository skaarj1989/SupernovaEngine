#include "MaterialEditor/MasterVariant.hpp"
#include "VisitorHelper.hpp"

const char *toString(const MasterNodeVariant &v) {
  return std::visit(
    Overload{
      [](const VertexMasterNode &) { return "VertexMaster"; },
      [](const SurfaceMasterNode &) { return "SurfaceMaster"; },
      [](const PostProcessMasterNode &) { return "PostProcessMaster"; },
    },
    v);
}
