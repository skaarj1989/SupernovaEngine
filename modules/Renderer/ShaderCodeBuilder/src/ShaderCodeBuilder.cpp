#include "ShaderCodeBuilder.hpp"
#include "os/FileSystem.hpp"
#include "spdlog/spdlog.h"
#include "tracy/Tracy.hpp"

#include <algorithm>
#include <regex>

namespace {

constexpr auto kNewLineCharacter = '\n';

void removeVersionDirective(std::string &sourceCode) {
  if (auto offset = sourceCode.find("#version"); offset != std::string::npos) {
    const auto endOfLinePos = sourceCode.find_first_of(kNewLineCharacter) + 1;
    sourceCode.replace(offset, endOfLinePos, "");
  }
}

// https://stackoverflow.com/questions/26492513/write-c-regular-expression-to-match-a-include-preprocessing-directive
// https://stackoverflow.com/questions/42139302/using-regex-to-filter-for-preprocessor-directives

[[nodiscard]] auto findIncludes(const std::string &src) {
  // Source: #include "filename"
  // [1] "
  // [2] filename
  // [3] "

  // TODO: Ignore directives inside a comment
  static const std::regex kIncludePattern(
    R"(#\s*include\s*([<"])([^>"]+)([>"]))");
  return std::sregex_iterator{src.cbegin(), src.cend(), kIncludePattern};
}

struct IncludeInfo {
  std::string path;
  bool relative;
};
[[nodiscard]] std::optional<IncludeInfo> statInclude(const std::smatch &m) {
  assert(m.size() == 4);
  const std::pair syntaxForm{*m[1].first, *m[3].first};
  const auto &path = m[2];
  if (syntaxForm == std::pair{'"', '"'}) {
    return IncludeInfo{.path = path.str(), .relative = true};
  } else if (syntaxForm == std::pair{'<', '>'}) {
    return IncludeInfo{.path = path.str(), .relative = false};
  }
  return std::nullopt;
}

auto eraseLine(std::string &str, std::size_t at) {
  const auto start = str.rfind(kNewLineCharacter, at) + 1;
  const auto end = str.find(kNewLineCharacter, start) + 1;
  const auto length = end - start;
  str.erase(start, length);
  return std::pair{start, length};
}

void resolveInclusions(std::string &src,
                       std::unordered_set<std::size_t> &includedPaths,
                       const std::filesystem::path &rootPath, int32_t level,
                       const std::filesystem::path &currentPath) {
  ZoneScopedN("#include");
  ptrdiff_t offset{0};

  // Copy to avoid dereference of invalidated string iterator.
  std::string temp{src};
  for (auto match = findIncludes(temp); match != std::sregex_iterator{};
       ++match) {
    const auto includeInfo = statInclude(*match);
    if (!includeInfo) {
      SPDLOG_WARN("Ill-formed directive: {}", match->begin()->str());
      offset -= eraseLine(src, match->position() + offset).second;
      continue;
    }
    const auto &[filename, isRelative] = *includeInfo;
    const auto next =
      ((isRelative ? currentPath : rootPath) / filename).lexically_normal();
    const auto hash = std::filesystem::hash_value(next);
    if (auto [_, inserted] = includedPaths.insert(hash); !inserted) {
      SPDLOG_TRACE("'{}' already included, skipping.", filename);
      offset -= eraseLine(src, match->position() + offset).second;
      continue;
    }

    std::string codeChunk;
    if (auto text = os::FileSystem::readText(next); text) {
      codeChunk = std::move(*text);
      resolveInclusions(codeChunk, includedPaths, rootPath, level + 1,
                        next.parent_path());
    } else {
      codeChunk = std::format(R"(#error "{} {}")", text.error(), filename);
    }

    auto [lineStartPos, lineLength] =
      eraseLine(src, match->position() + offset);
    src.insert(lineStartPos, codeChunk);
    offset += codeChunk.length() - lineLength;
  }
}

void insert(auto &dst, const auto &src) {
  dst.insert(dst.end(), std::cbegin(src), std::cend(src));
}

} // namespace

//
// ShaderCodeBuilder class:
//

ShaderCodeBuilder::ShaderCodeBuilder(const std::filesystem::path &rootPath)
    : m_rootPath{rootPath} {}

ShaderCodeBuilder &ShaderCodeBuilder::include(const std::filesystem::path &p) {
  m_includes.emplace(p.lexically_normal().generic_string());
  return *this;
}

ShaderCodeBuilder &ShaderCodeBuilder::clearDefines() {
  m_defines.clear();
  return *this;
}
ShaderCodeBuilder &ShaderCodeBuilder::setDefines(const Defines &defines) {
  m_defines = defines;
  return *this;
}
ShaderCodeBuilder &
ShaderCodeBuilder::addDefines(std::initializer_list<const std::string> v) {
  insert(m_defines, v);
  return *this;
}
ShaderCodeBuilder &
ShaderCodeBuilder::addDefines(std::span<const std::string> v) {
  insert(m_defines, v);
  return *this;
}
ShaderCodeBuilder &ShaderCodeBuilder::addDefine(const std::string &s) {
  m_defines.emplace_back(s);
  return *this;
}

ShaderCodeBuilder &ShaderCodeBuilder::replace(const std::string &phrase,
                                              const std::string_view s) {
  m_patches.insert_or_assign(phrase, s);
  return *this;
}

std::string
ShaderCodeBuilder::buildFromFile(const std::filesystem::path &p) const {
  const auto filePath = m_rootPath / p;
  auto str = os::FileSystem::readText(filePath);
  return str ? buildFromString(*str, filePath.parent_path()) : "";
}

std::string
ShaderCodeBuilder::buildFromString(std::string sourceCode,
                                   const std::filesystem::path &origin) const {
  ZoneScopedN("BuildShaderCode");
  removeVersionDirective(sourceCode);

  std::ostringstream oss;

  std::ranges::transform(
    m_defines, std::ostream_iterator<std::string>{oss, "\n"},
    [](const auto &s) { return std::format("#define {}", s); });
  std::ranges::transform(
    m_includes, std::ostream_iterator<std::string>{oss, "\n"},
    [](const auto &s) { return std::format(R"(#include "{}")", s); });

  if (oss.tellp() > 0) {
    const auto lastExtensionPos = sourceCode.rfind("#extension");
    const auto extraCodeLine =
      sourceCode.find_first_of(kNewLineCharacter, lastExtensionPos) + 1;
    sourceCode.insert(extraCodeLine, oss.str());
  }

  std::unordered_set<std::size_t> includedPaths;
  resolveInclusions(sourceCode, includedPaths, m_rootPath, 0,
                    m_rootPath == origin ? m_rootPath : m_rootPath / origin);

  for (const auto &[phrase, patch] : m_patches) {
    if (const auto pos = sourceCode.find(phrase); pos != std::string::npos)
      sourceCode.replace(pos, phrase.length(), patch);
  }
  return sourceCode;
}
