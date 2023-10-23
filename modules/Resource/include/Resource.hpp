#pragma once

#include "entt/core/fwd.hpp"
#include "entt/resource/resource.hpp"
#include "entt/locator/locator.hpp"
#include <filesystem>
#include <optional>

class Resource {
public:
  static const entt::id_type kInvalidId;

  Resource() = default;
  explicit Resource(const std::filesystem::path &);
  Resource(const Resource &) = delete;
  Resource(Resource &&) noexcept = default;
  virtual ~Resource() = default;

  Resource &operator=(const Resource &) = delete;
  Resource &operator=(Resource &&) noexcept = default;

  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] entt::id_type getResourceId() const;
  [[nodiscard]] bool isVirtual() const;
  [[nodiscard]] const std::filesystem::path &getPath() const;

  [[nodiscard]] bool operator==(const Resource &) const;
  [[nodiscard]] bool operator==(const std::filesystem::path &) const;

private:
  entt::id_type m_id{kInvalidId};
  bool m_virtual{true};
  std::filesystem::path m_path;
};

[[nodiscard]] std::optional<std::string> serialize(const Resource &);

template <typename T>
std::optional<std::string> serialize(const std::shared_ptr<T> &sp) {
  auto r = std::dynamic_pointer_cast<const Resource>(sp);
  return r ? serialize(*r) : std::nullopt;
}

[[nodiscard]] bool isValid(const Resource *resource);
[[nodiscard]] bool isValid(const entt::resource<Resource> &);

[[nodiscard]] std::string toString(const Resource &);

[[nodiscard]] entt::id_type makeResourceId(const std::filesystem::path &);

template <typename Manager> auto loadResourceHandle(const std::string_view p) {
  return entt::locator<Manager>::value().load(p);
}
template <typename Manager> auto loadResource(const std::string_view p) {
  return loadResourceHandle<Manager>(p).handle();
}
