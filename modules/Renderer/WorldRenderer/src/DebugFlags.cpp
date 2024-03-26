#include "renderer/DebugFlags.hpp"
#include "StringUtility.hpp"
#include <vector>

namespace gfx {

std::string toString(const DebugFlags flags) {
  using enum DebugFlags;
  if (flags == None) return "None";

  std::vector<const char *> values;
  constexpr auto kMaxNumFlags = 8;
  values.reserve(kMaxNumFlags);

#define CHECK_FLAG(Value)                                                      \
  if (bool(flags & Value)) values.push_back(#Value)

  CHECK_FLAG(WorldBounds);
  CHECK_FLAG(InfiniteGrid);
  CHECK_FLAG(Wireframe);
  CHECK_FLAG(VertexNormal);
  CHECK_FLAG(CascadeSplits);
  CHECK_FLAG(LightHeatmap);
  CHECK_FLAG(VPL);
  CHECK_FLAG(IrradianceOnly);

  return join(values, ", ");
}

} // namespace gfx
