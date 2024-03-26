#include "rhi/ImageUsage.hpp"
#include "StringUtility.hpp"
#include <vector>

namespace rhi {

std::string toString(const ImageUsage flags) {
  std::vector<const char *> values;
  constexpr auto kMaxNumFlags = 5;
  values.reserve(kMaxNumFlags);

#define CHECK_FLAG(Value)                                                      \
  if (bool(flags & ImageUsage::Value)) values.push_back(#Value)

  CHECK_FLAG(TransferSrc);
  CHECK_FLAG(TransferDst);
  CHECK_FLAG(Storage);
  CHECK_FLAG(RenderTarget);
  CHECK_FLAG(Sampled);

#undef CHECK_FLAG

  return join(values, ", ");
}

} // namespace rhi
