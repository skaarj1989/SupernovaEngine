#include "MeshCache.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "CacheInspector.hpp"

namespace {

void listAttributes(const rhi::VertexAttributes &attributes) {
  for (const auto &[location, _] : attributes)
    ImGui::BulletText("%s",
                      toString(static_cast<gfx::AttributeLocation>(location)));
}
void print(const gfx::VertexFormat &vertexFormat) {
  ImGui::Text("VertexFormat: %zu", vertexFormat.getHash());
  ImGui::SeparatorText("Attributes");
  listAttributes(vertexFormat.getAttributes());
}
void print(const AABB &aabb) {
  ImGui::TextUnformatted("AABB:");
  if (isUninitialized(aabb)) {
    ImGui::SameLine();
    ImGui::TextUnformatted("(uninitialized)");
  } else {
#if 1
    const auto center = aabb.getCenter();
    ImGui::BulletText("center = [%.2f, %.2f, %.2f]", center.x, center.y,
                      center.z);
    const auto extent = aabb.getExtent();
    ImGui::BulletText("extent = [%.2f, %.2f, %.2f]", extent.x, extent.y,
                      extent.z);
    ImGui::BulletText("radius = %.2f", aabb.getRadius());
#else
    ImGui::BulletText("min = [%.2f, %.2f, %.2f]", aabb.min.x, aabb.min.y,
                      aabb.min.z);
    ImGui::BulletText("max = [%.2f, %.2f, %.2f]", aabb.max.x, aabb.max.y,
                      aabb.max.z);
#endif
  }
}
void print(const gfx::Mesh &mesh) {
  print(mesh.getVertexFormat());
  ImGui::Separator();
  ImGui::Text("Num submeshes: %zu", mesh.getSubMeshes().size());
  ImGui::Separator();
  print(mesh.getAABB());
}

} // namespace

void show(const char *name, bool *open, gfx::MeshManager &cache) {
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    defaultMenuBar(cache);
    view(
      cache,
      [](auto id) {
        onDragSource(kImGuiPayloadTypeMesh, id,
                     [] { ImGui::TextUnformatted("Mesh inside ..."); });
      },
      [](const auto &r) { print(r); },
      [&cache](auto id) { return !cache.isBuiltIn(id); });
  }
  ImGui::End();
}
