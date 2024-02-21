#pragma once

#include "fg/FrameGraph.hpp"
#include "TransientBuffer.hpp"
#include "FrameGraphResourceAccess.hpp"
#include "RenderContext.hpp"

namespace gfx {

template <typename T>
[[nodiscard]] std::optional<FrameGraphResource>
uploadContainer(FrameGraph &fg, const std::string_view passName,
                TransientBuffer<std::vector<T>> &&info) {
  if (info.data.empty()) return std::nullopt;

  ZoneScoped;

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
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, passName.data())
      rc.commandBuffer.update(
        *resources.get<FrameGraphBuffer>(data.buffer).buffer, 0, dataSize,
        v.data());
    });

  return buffer;
}

} // namespace gfx
