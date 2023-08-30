#pragma once

#include "rhi/ShaderType.hpp"
#include "rhi/SPIRV.hpp"
#include <string>
#include <expected>

namespace rhi {

class ShaderCompiler final {
public:
  ShaderCompiler();
  ShaderCompiler(const ShaderCompiler &) = delete;
  ShaderCompiler(ShaderCompiler &&) noexcept = delete;
  ~ShaderCompiler();

  ShaderCompiler &operator=(const ShaderCompiler &) = delete;
  ShaderCompiler &operator=(ShaderCompiler &&) noexcept = delete;

  [[nodiscard]] std::expected<SPIRV, std::string>
  compile(ShaderType, const std::string_view) const;
};

} // namespace rhi
