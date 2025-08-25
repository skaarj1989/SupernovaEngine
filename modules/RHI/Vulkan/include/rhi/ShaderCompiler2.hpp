#pragma once

#include "ShaderType.hpp"
#include "SPIRV.hpp"
#include "SlangCompilationRequest.hpp"

#include <slang.h>
#include <slang-com-ptr.h>
#include <nlohmann/json.hpp>

#include <expected>

namespace rhi {

class ShaderCompiler2 {
public:
  ShaderCompiler2();

  ShaderCompiler2(const ShaderCompiler2 &) = delete;
  ShaderCompiler2(ShaderCompiler2 &&) noexcept = delete;
  ~ShaderCompiler2();

  ShaderCompiler2 &operator=(const ShaderCompiler2 &) = delete;
  ShaderCompiler2 &operator=(ShaderCompiler2 &&) noexcept = delete;

  using ErrorMessage = std::string;
  struct Data {
    SPIRV code;
    nlohmann::json reflection;
  };
  using Result = std::expected<Data, ErrorMessage>;

  [[nodiscard]] Result compile(const SlangCompilationRequest &) const;

private:
  [[nodiscard]] Slang::ComPtr<slang::ISession>
  _createSession(const SlangCompilationRequest::Defines &) const;

private:
  Slang::ComPtr<slang::IGlobalSession> m_globalSession;
};

using EntryPoints = std::unordered_map<ShaderType, std::string>;

[[nodiscard]] EntryPoints queryEntryPoints(const nlohmann::json &reflection);

} // namespace rhi
