#include "renderer/RenderFeatures.hpp"
#include "StringUtility.hpp"
#include <vector>

namespace gfx {

std::string toString(const RenderFeatures flags) {
  using enum RenderFeatures;
  if (flags == None) return "None";

  std::vector<const char *> values;
  constexpr auto kMaxNumFlags = 9;
  values.reserve(kMaxNumFlags);

#define CHECK_FLAG(Value)                                                      \
  if (bool(flags & Value)) values.push_back(#Value)

  CHECK_FLAG(LightCulling);
  CHECK_FLAG(SoftShadows);
  CHECK_FLAG(GI);
  CHECK_FLAG(SSAO);
  CHECK_FLAG(SSR);
  CHECK_FLAG(Bloom);
  CHECK_FLAG(FXAA);
  CHECK_FLAG(EyeAdaptation);
  CHECK_FLAG(CustomPostprocess);

  return join(values, ", ");
}

} // namespace gfx
