#pragma once

#include "fg/FrameGraph.hpp"
#include "TransientBuffer.hpp"
#include "FrameGraphResourceAccess.hpp"
#include "RenderContext.hpp"

namespace gfx {

template <typename T>
[[nodiscard]] FrameGraphResource uploadStruct(FrameGraph &fg,
                                              const std::string_view passName,
                                              TransientBuffer<T> &&s) {
  ZoneScoped;

  constexpr auto kDataSize = uint32_t(sizeof(T));

  struct Data {
    FrameGraphResource buffer;
  };
  const auto [buffer] = fg.addCallbackPass<Data>(
    passName,
    [&s](FrameGraph::Builder &builder, Data &data) {
      data.buffer =
        builder.create<FrameGraphBuffer>(s.name, {
                                                   .type = s.type,
                                                   .stride = kDataSize,
                                                   .capacity = 1,
                                                 });
      data.buffer = builder.write(
        data.buffer, BindingInfo{.pipelineStage = PipelineStage::Transfer});
    },
    [passName, s = std::move(s.data)](
      const Data &data, FrameGraphPassResources &resources, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, passName.data())
      rc.commandBuffer.update(
        *resources.get<FrameGraphBuffer>(data.buffer).buffer, 0, kDataSize, &s);
    });

  return buffer;
}

} // namespace gfx
