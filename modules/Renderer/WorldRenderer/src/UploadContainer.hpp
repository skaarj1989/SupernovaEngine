#pragma once

#include "fg/FrameGraph.hpp"
#include "TransientBuffer.hpp"
#include "FrameGraphResourceAccess.hpp"
#include "RenderContext.hpp"
#include "rhi/CommandBuffer.hpp"

namespace gfx {

template <typename T>
[[nodiscard]] std::optional<FrameGraphResource>
uploadContainer(FrameGraph &fg, const std::string_view passName,
                TransientBuffer<std::vector<T>> &&info) {
  if (info.data.empty()) return std::nullopt;

  ZoneTransientN(__tracy_zone, passName.data(), true);

  constexpr auto kStride = sizeof(T);
  const auto capacity = info.data.size();
  const auto dataSize = kStride * capacity;
  assert(dataSize > 0);

  struct Data {
    FrameGraphResource buffer;
  };
  const auto [buffer] = fg.addCallbackPass<Data>(
    passName,
    [&info, capacity](FrameGraph::Builder &builder, Data &data) {
      PASS_SETUP_ZONE;

      data.buffer =
        builder.create<FrameGraphBuffer>(info.name, {
                                                      .type = info.type,
                                                      .stride = kStride,
                                                      .capacity = capacity,
                                                    });
      data.buffer = builder.write(
        data.buffer, BindingInfo{.pipelineStage = PipelineStage::Transfer});
    },
    [passName, v = std::move(info.data), dataSize](
      const Data &data, FrameGraphPassResources &resources, void *ctx) {
      auto &cb = static_cast<RenderContext *>(ctx)->commandBuffer;
      RHI_GPU_ZONE(cb, passName.data());
      cb.update(*resources.get<FrameGraphBuffer>(data.buffer).buffer, 0,
                dataSize, v.data());
    });

  return buffer;
}

} // namespace gfx
