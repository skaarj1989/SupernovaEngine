#pragma once

#include <filesystem>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <span>

using Defines = std::vector<std::string>;

class ShaderCodeBuilder {
public:
  explicit ShaderCodeBuilder(const std::filesystem::path & = "./shaders");
  ShaderCodeBuilder(const ShaderCodeBuilder &) = delete;
  ShaderCodeBuilder(ShaderCodeBuilder &&) noexcept = delete;
  ~ShaderCodeBuilder() = default;

  ShaderCodeBuilder &operator=(const ShaderCodeBuilder &) = delete;
  ShaderCodeBuilder &operator=(ShaderCodeBuilder &&) noexcept = delete;

  // ---

  ShaderCodeBuilder &include(const std::filesystem::path &);

  ShaderCodeBuilder &clearDefines();
  ShaderCodeBuilder &setDefines(const Defines &);
  ShaderCodeBuilder &addDefines(std::initializer_list<const std::string>);
  ShaderCodeBuilder &addDefines(std::span<const std::string>);
  ShaderCodeBuilder &addDefine(const std::string &);
  template <typename T> ShaderCodeBuilder &addDefine(const std::string &, T &&);

  ShaderCodeBuilder &replace(const std::string &phrase, const std::string_view);

  [[nodiscard]] std::string buildFromFile(const std::filesystem::path &) const;
  [[nodiscard]] std::string
  buildFromString(std::string sourceCode,
                  const std::filesystem::path &origin = "") const;

private:
  std::filesystem::path m_rootPath;

  Defines m_defines;
  std::unordered_set<std::string> m_includes;
  std::unordered_map<std::string, std::string> m_patches;
};

#include "ShaderCodeBuilder.inl"
