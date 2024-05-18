#include "WorldRendererWidget.hpp"
#include "rhi/RenderDevice.hpp"
#include "renderer/WorldRenderer.hpp"
#include "IconsFontAwesome6.h"
#include "implot.h"

namespace {

[[nodiscard]] auto toString(const gfx::PipelineGroups groups) {
  switch (groups) {
    using enum gfx::PipelineGroups;

  case SurfaceMaterial:
    return "Surface";
  case PostProcessMaterial:
    return "PostProcess";
  case BuiltIn:
    return "BuiltIn";

  default:
    assert(false);
    return "";
  }
}

using Measure = std::pair<const char *, double>;
// clang-format off
static const auto kMeasures = std::array{
#if 1
  Measure{"G", 1E9},  // 10^9
  Measure{"M", 1E6},  // 10^6
  Measure{"K", 1E3},  // 10^3
  Measure{ "", 1},
#else 
  Measure{"Gi", 1 << 30}, // 2^30
  Measure{"Mi", 1 << 20}, // 2^20
  Measure{"Ki", 1 << 10}, // 2^10
  Measure{  "", 1},
#endif
};
// clang-format on
int metricFormatter(double value, char *buff, int size, void *data) {
  const auto *unit = static_cast<const char *>(data);
  if (value == 0) return snprintf(buff, size, "0 %s", unit);

  for (const auto [prefix, divisor] : kMeasures) {
    if (fabs(value) >= divisor)
      return snprintf(buff, size, "%g %s%s", value / divisor, prefix, unit);
  }
  return snprintf(buff, size, "%g %s%s", value / kMeasures.back().second,
                  kMeasures.back().first, unit);
}

} // namespace

//
// ScrollingBuffer struct:
//

WorldRendererWidget::ScrollingBuffer::ScrollingBuffer(const int32_t capacity_)
    : capacity{capacity_} {
  data.reserve(capacity);
}
void WorldRendererWidget::ScrollingBuffer::addPoint(const float t,
                                                    const float value) {
  if (data.size() < capacity) {
    data.push_back({t, value});
  } else {
    data[offset] = {t, value};
    offset = (offset + 1) % capacity;
  }
}

//
// WorldRendererWidget class:
//

WorldRendererWidget::WorldRendererWidget(gfx::WorldRenderer &renderer)
    : m_renderer{renderer} {}

void WorldRendererWidget::show(const char *name, bool *open) {
  m_time += ImGui::GetIO().DeltaTime;

  if (ImGui::Begin(name, open)) {
    ZoneScopedN("WorldRendererWindow");
    if (ImGui::CollapsingHeader("Pipelines", ImGuiTreeNodeFlags_DefaultOpen)) {
      constexpr auto kTableFlags =
        ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH;
      if (ImGui::BeginTable(IM_UNIQUE_ID, 3, kTableFlags)) {
        ImGui::TableSetupColumn("Group");
        ImGui::TableSetupColumn("Count");
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_NoHeaderLabel);

        ImGui::TableHeadersRow();

        using enum gfx::PipelineGroups;
        for (const auto group :
             {SurfaceMaterial, PostProcessMaterial, BuiltIn}) {
          ImGui::TableNextRow();

          ImGui::TableSetColumnIndex(0);
          ImGui::TextUnformatted(toString(group));

          ImGui::TableSetColumnIndex(1);
          const auto cacheSize = m_renderer.countPipelines(group);
          ImGui::Text("%u", cacheSize);

          ImGui::TableSetColumnIndex(2);
          ImGui::PushID(std::to_underlying(group));
          ImGui::BeginDisabled(cacheSize == 0);
          if (ImGui::SmallButton(ICON_FA_ERASER)) {
            m_renderer.getRenderDevice().waitIdle();
            m_renderer.clearPipelines(group);
          }
          ImGui::EndDisabled();
          ImGui::PopID();
        }
        ImGui::EndTable();
      }
    }
    if (ImGui::CollapsingHeader("TransientResources",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      const auto stats = m_renderer.getTransientResourcesStats();
      m_textures.addPoint(m_time, static_cast<float>(stats.textures));
      m_buffers.addPoint(m_time, static_cast<float>(stats.buffers));

      constexpr auto kHistory = 10.0f;

      if (ImPlot::BeginPlot(IM_UNIQUE_ID)) {
        ImPlot::SetupAxes("Time", "Size", ImPlotAxisFlags_NoLabel,
                          ImPlotAxisFlags_LockMin | ImPlotAxisFlags_NoLabel);

        ImPlot::SetupAxisFormat(ImAxis_X1, "%g s");
        ImPlot::SetupAxisLimits(ImAxis_X1, std::max(0.0f, m_time - kHistory),
                                m_time, ImPlotCond_Always);

        ImPlot::SetupAxisFormat(ImAxis_Y1, metricFormatter, (void *)"B");
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, kMeasures[1].second * 256,
                                ImPlotCond_Once);

        static const auto plotLine = [](const char *label,
                                        const ScrollingBuffer &buffer) {
          ImPlot::PlotLine(label, &buffer.data[0].x, &buffer.data[0].y,
                           buffer.data.size(), ImPlotLineFlags_None,
                           buffer.offset, sizeof(ImVec2));
        };
        plotLine("Textures", m_textures);
        plotLine("Buffers", m_buffers);

        ImPlot::EndPlot();
      }
    }
  }
  ImGui::End();
}
