#pragma once

#include "ShaderType.hpp"
#include "SPIRV.hpp"
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

  using ErrorMessage = std::string;
  using Result = std::expected<SPIRV, ErrorMessage>;

  [[nodiscard]] Result compile(const ShaderType,
                               const std::string_view code) const;
};

} // namespace rhi
