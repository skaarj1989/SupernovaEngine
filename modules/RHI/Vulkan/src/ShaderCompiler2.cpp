#include "rhi/ShaderCompiler2.hpp"
#include <array>

namespace rhi {

namespace {

[[nodiscard]] std::string toString(slang::IBlob *blob) {
  if (!blob) return {};

  return std::string{static_cast<const char *>(blob->getBufferPointer()),
                     blob->getBufferSize()};
}

[[nodiscard]] SPIRV toSPIRV(slang::IBlob *blob) {
  if (!blob) return {};

  const auto wordCount = blob->getBufferSize() / sizeof(uint32_t);
  const auto data = static_cast<const uint32_t *>(blob->getBufferPointer());
  return SPIRV{data, data + wordCount};
}

constexpr auto kTargetIndex = 0;

} // namespace

//
// ShaderCompiler2 class:
//

ShaderCompiler2::ShaderCompiler2() {
  const SlangGlobalSessionDesc desc{
    .minLanguageVersion = SLANG_LANGUAGE_VERSION_LATEST,
  };
  slang::createGlobalSession(&desc, m_globalSession.writeRef());
}
ShaderCompiler2::~ShaderCompiler2() = default;

ShaderCompiler2::Result
ShaderCompiler2::compile(const SlangCompilationRequest &request) const {
  Slang::ComPtr<slang::IBlob> diagnostics;

#define CHECK_RESULT(res)                                                      \
  if (!res) return std::unexpected {                                           \
      toString(diagnostics)                                                    \
    }

  auto session = _createSession(request.defines);

  auto *module =
    session->loadModule(request.moduleName.c_str(), diagnostics.writeRef());
  CHECK_RESULT(module);

  const auto numEntryPoints = module->getDefinedEntryPointCount();
  std::vector<slang::IComponentType *> componentsToLink;
  componentsToLink.reserve(numEntryPoints + 1);
  componentsToLink.push_back(module);

  for (SlangInt32 i = 0; i < numEntryPoints; ++i) {
    slang::IEntryPoint *entryPoint{nullptr};
    if (module->getDefinedEntryPoint(i, &entryPoint) == SLANG_OK)
      componentsToLink.push_back(entryPoint);
  }

  Slang::ComPtr<slang::IComponentType> composed;
  auto result = session->createCompositeComponentType(
    componentsToLink.data(), componentsToLink.size(), composed.writeRef(),
    diagnostics.writeRef());
  CHECK_RESULT(result == SLANG_OK);

  Slang::ComPtr<slang::IComponentType> program;
  result = composed->link(program.writeRef(), diagnostics.writeRef());
  CHECK_RESULT(result == SLANG_OK);

  Slang::ComPtr<slang::IBlob> spirv;
  result = composed->getTargetCode(kTargetIndex, spirv.writeRef(),
                                   diagnostics.writeRef());
  CHECK_RESULT(result == SLANG_OK);

  Slang::ComPtr<slang::IBlob> reflection;
  result = composed->getLayout()->toJson(reflection.writeRef());
  CHECK_RESULT(result == SLANG_OK);

  return Data{
    .code = toSPIRV(spirv),
    .reflection = nlohmann::json::parse(toString(reflection)),
  };
}

Slang::ComPtr<slang::ISession> ShaderCompiler2::_createSession(
  const SlangCompilationRequest::Defines &defines) const {
  const slang::TargetDesc targetTesc{
    .format = SLANG_SPIRV,
    // https://shader-slang.org/slang/user-guide/a3-01-reference-capability-profiles.html
    .profile = m_globalSession->findProfile("spirv_1_5"),
  };

  const auto searchPaths = std::array{"./shaders"};
  std::vector<slang::PreprocessorMacroDesc> preprocessorMacros{
    {"DEPTH_ZERO_TO_ONE", "1"},
  };
  for (const auto &[key, value] : defines)
    preprocessorMacros.push_back({key.c_str(), value.c_str()});

  const slang::SessionDesc sessionDesc{
    .targets = &targetTesc,
    .targetCount = 1,

    .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,

    .searchPaths = searchPaths.data(),
    .searchPathCount = searchPaths.size(),

    .preprocessorMacros = preprocessorMacros.data(),
    .preprocessorMacroCount = static_cast<SlangInt>(preprocessorMacros.size()),
  };

  Slang::ComPtr<slang::ISession> session;
  m_globalSession->createSession(sessionDesc, session.writeRef());
  return session;
}

static void from_json(const nlohmann::json &j, ShaderType &out) {
  using enum ShaderType;

  if (j == "vertex")
    out = Vertex;
  else if (j == "geometry")
    out = Geometry;
  else if (j == "fragment" || j == "pixel")
    out = Fragment;
  else if (j == "compute")
    out = Compute;
}
static void from_json(const nlohmann::json &j, EntryPoints &out) {
  if (!j.contains("entryPoints")) return;

  for (const auto &entryPoint : j["entryPoints"]) {
    out[entryPoint.at("stage").get<ShaderType>()] =
      entryPoint.at("name").get<std::string>();
  }
}

EntryPoints queryEntryPoints(const nlohmann::json &j) {
  EntryPoints out;
  j.get_to<EntryPoints>(out);
  return out;
}

} // namespace rhi
