#include "Resource.hpp"
#include "os/FileSystem.hpp"
#include "entt/core/hashed_string.hpp"

const entt::id_type Resource::kInvalidId = entt::hashed_string{""}.value();

Resource::Resource(const std::filesystem::path &p)
    : m_id{makeResourceId(p)}, m_virtual{!std::filesystem::is_regular_file(p)},
      m_path{p} {}

Resource::operator bool() const { return m_id != kInvalidId; }

entt::id_type Resource::getResourceId() const { return m_id; }
bool Resource::isVirtual() const { return m_virtual; }
const std::filesystem::path &Resource::getPath() const { return m_path; }

bool Resource::operator==(const std::filesystem::path &p) const {
  return m_id == makeResourceId(p);
}
bool Resource::operator==(const Resource &other) const {
  return m_id == other.m_id;
}

//
// Utility:
//

std::optional<std::string> serialize(const Resource &r) {
  if (!r) return std::nullopt;

  const auto &p = r.getPath();
  return (r.isVirtual() ? p : *os::FileSystem::relativeToRoot(p)).string();
}

bool isValid(const Resource *resource) { return resource && bool(*resource); }
bool isValid(const entt::resource<Resource> &resource) {
  return resource && isValid(resource.handle().get());
}

std::string toString(const Resource &r) {
  return r.getPath().filename().string();
}

entt::id_type makeResourceId(const std::filesystem::path &p) {
  const auto str =
    (std::filesystem::is_regular_file(p) ? std::filesystem::absolute(p) : p)
      .string();
  return entt::hashed_string{str.c_str()};
}
