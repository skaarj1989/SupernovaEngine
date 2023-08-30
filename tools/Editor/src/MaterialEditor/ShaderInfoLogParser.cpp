#include "ShaderInfoLogParser.hpp"
#include "StringUtility.hpp"

#include <vector>
#include <optional>

namespace {

[[nodiscard]] auto tokenize(const std::string_view str, char delimiter,
                            const std::optional<int32_t> depth = std::nullopt) {
  std::vector<std::string> tokens;

  std::istringstream iss{str.data()};
  std::string token;

  auto i = 0;
  while (const auto &is = std::getline(iss, token, delimiter)) {
    tokens.emplace_back(std::move(token));
    ++i;

    if (depth && *depth == i) {
      // False-positive C26800. The "token" is an output for "iss".
      std::getline(iss, token);
      tokens.emplace_back(std::move(token));
      break;
    }
  }
  return tokens;
}
[[nodiscard]] auto getLines(const std::string_view str) {
  return tokenize(str, '\n');
}

struct Entry {
  int32_t fileIndex;
  int32_t lineIndex;
  std::string message;
};
[[nodiscard]] std::optional<Entry> parseLine(const std::string_view line) {
  // ERROR: 0:1: 'foo' : undeclared identifier
  auto tokens = tokenize(line, ':', 3);

  // [0] = "ERROR"
  constexpr auto kFileIndex = 1;
  constexpr auto kLineIndex = 2;
  // [3] = error message

  if (tokens.size() < 4) return std::nullopt;

  Entry entry{
    .fileIndex = std::stoi(tokens[kFileIndex]),
    .lineIndex = std::stoi(tokens[kLineIndex]),
    .message = std::move(tokens.back()),
  };
  trim(entry.message);
  return entry;
}

using LineError = std::map<int32_t, std::vector<std::string>>;
using FileError = std::map<int32_t, LineError>;

[[nodiscard]] auto parse(const std::string_view infoLog) {
  auto lines = getLines(infoLog);
  std::erase_if(lines, [](const auto &str) { return str.empty(); });
  lines.pop_back(); // Summary message.

  FileError errors;
  for (const auto &line : lines) {
    if (auto e = parseLine(line); e) {
      const auto &[fileIndex, lineIndex, message] = *e;
      errors[fileIndex][lineIndex].push_back(message);
    }
  }
  return errors;
}

} // namespace

ErrorMarkers getErrorMarkers(const std::string_view infoLog) {
  const auto files = parse(infoLog);
  const auto &mainFileErrors = files.begin()->second;

  ErrorMarkers errorMarkers;
  std::ranges::transform(mainFileErrors,
                         std::inserter(errorMarkers, errorMarkers.end()),
                         [](const auto &p) {
                           const auto &[line, strings] = p;
                           return std::pair{line, join(strings, "\n")};
                         });
  return errorMarkers;
}
