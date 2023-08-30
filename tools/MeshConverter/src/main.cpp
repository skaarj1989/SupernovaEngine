#include "ParserActions.hpp"

#include "assimp/postprocess.h"
#include "assimp/DefaultLogger.hpp"
#include "glm2ai.hpp"

#include "PrintSceneStats.hpp"
#include "ExtraAnimations.hpp"
#include "SupernovaMeshExport.hpp"

#include "spdlog/spdlog.h"

#include <expected>

namespace {

class CustomLogger : public Assimp::Logger {
public:
  explicit CustomLogger(spdlog::logger &logger) : m_logger{logger} {}

  void OnDebug(const char *message) override { m_logger.debug(message); }
  void OnVerboseDebug(const char *message) override { m_logger.debug(message); }
  void OnInfo(const char *message) override { m_logger.info(message); }
  void OnWarn(const char *message) override { m_logger.warn(message); }
  void OnError(const char *message) override { m_logger.error(message); }

  bool attachStream(Assimp::LogStream *, uint32_t) override { return false; }
  bool detachStream(Assimp::LogStream *, uint32_t) override { return false; }

private:
  spdlog::logger &m_logger;
};

class Stopwatch {
  using clock = std::chrono::high_resolution_clock;

public:
  Stopwatch() { start(); }

  void start() { m_start = clock::now(); }
  template <typename T = std::chrono::milliseconds> auto count() {
    return std::chrono::duration_cast<T>(clock::now() - m_start);
  }

private:
  clock::time_point m_start;
};

class App {
public:
  App() : m_program{_prepareParser()} {
    auto &logger = *spdlog::default_logger();
    logger.set_pattern("[%^%l%$] %v");
    logger.set_level(spdlog::level::debug);
    m_logger = std::make_unique<CustomLogger>(logger);
  }

  int32_t execute(int argc, char *argv[]) {
    try {
      m_program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
      std::cerr << err.what() << std::endl << m_program;
      return -1;
    }

    const std::filesystem::path resourcePath =
      m_program.get<std::string>("input");

    if (m_program["--verbose"] == true) {
      Assimp::DefaultLogger::set(m_logger.get());
    }

    if (const auto time = _load(resourcePath); time) {
      m_logger->info("Import time: ", *time);
    } else {
      std::cerr << "Importer error: " << time.error() << std::endl;
      return -2;
    }

    auto masterScene = std::unique_ptr<aiScene>(m_importer.GetOrphanedScene());

    if (m_program.present("--extra-animations")) {
      auto animations = loadExtraAnimations(
        m_importer, resourcePath.parent_path(),
        m_program.get<std::vector<std::string>>("--extra-animations"));
      copyAnimations(masterScene.get(), animations);
    }
    if (m_program["--stats"] == true) std::cout << *masterScene;

    if (const auto time = _export(resourcePath, *masterScene); time) {
      m_logger->info("Export time: ", *time);
    } else {
      std::cerr << "Exporter error: " << time.error() << std::endl;
      return -3;
    }

    return 0;
  }

private:
  [[nodiscard]] argparse::ArgumentParser _prepareParser() const {
    argparse::ArgumentParser parser{"MeshConverter"};
    parser.add_argument("input")
      .help("Input resource file")
      .metavar("RESOURCE_PATH")
      .required();

    parser.add_argument("--verbose").default_value(false).implicit_value(true);
    parser.add_argument("--stats")
      .help("Prints file stats")
      .default_value(false)
      .implicit_value(true);

    // Transform:

    parser.add_argument("--translate")
      .help(
        "Position as vec3, comma separated values without whitespace: x,y,z")
      .metavar("VEC3")
      .action(parseVec3);
    parser.add_argument("--rotate")
      .help("Rotation as vec3 (Euler angles, in degrees), comma separated "
            "values without whitespace: x,y,z")
      .metavar("VEC3")
      .action(parseVec3);
    parser.add_argument("--scale")
      .help("Scale as vec3, comma separated values without whitespace: x,y,z")
      .metavar("VEC3")
      .action(parseVec3);

    // aiPostProcessSteps flags:

    parser.add_argument("--pre-transform-vertices")
      .help("Use if input contains instanced meshes")
      .default_value(false)
      .implicit_value(true);

    parser.add_argument("--smooth-normals")
      .help("Generates smooth normals")
      .default_value(false)
      .implicit_value(true);
    parser.add_argument("--calculate-tangents")
      .help("Generates tangent space")
      .default_value(false)
      .implicit_value(true);

    // Exporter internal flags:

    parser.add_argument("--ignore-materials")
      .default_value(false)
      .implicit_value(true);
    parser.add_argument("--generate-lods")
      .default_value(false)
      .implicit_value(true);

    // ---

    parser.add_argument("--extra-animations")
      .help("Load extra animations separated into multiple files")
      .metavar("RELATIVE_PATH")
      .remaining();

    return parser;
  }

  std::expected<std::chrono::milliseconds, std::string>
  _load(const std::filesystem::path &p) {
    Stopwatch stopwatch;

    const auto scene = m_importer.ReadFile(p.string(), 0);
    if (!scene) {
      return std::unexpected{m_importer.GetErrorString()};
    }
    if (const auto transform = getTransform(m_program); transform) {
      m_logger->info("Applying transform");
      scene->mRootNode->mTransformation = to_mat4(*transform);
    }

    // Defaults:
    // - Right-handed coordinate space.
    // - Face winding order is counter clockwise (CCW).

    auto flags = _getPostProcessingFlags(m_program);
    flags |= aiProcess_Triangulate;
    flags |= aiProcess_JoinIdenticalVertices; // Slow
    flags |= aiProcess_FlipUVs;
    m_importer.ApplyPostProcessing(flags);

    return stopwatch.count();
  }
  [[nodiscard]] int32_t
  _getPostProcessingFlags(const argparse::ArgumentParser &program) const {
    int32_t flags{0};
    if (program["--smooth-normals"] == true) {
      flags |= aiProcess_GenSmoothNormals;
    }
    if (program["--calculate-tangents"] == true) {
      flags |= aiProcess_CalcTangentSpace;
    }
    if (program["--pre-transform-vertices"] == true) {
      flags |= aiProcess_PreTransformVertices;
    }
    return flags;
  }

  std::expected<std::chrono::milliseconds, std::string>
  _export(const std::filesystem::path &p, const aiScene &scene) {
    Stopwatch stopwatch;

    Assimp::Exporter exporter;
    exporter.RegisterExporter(
      {"sne", "Supernova Mesh", "mesh", supernovaMeshExportFunction});

    Assimp::ExportProperties properties;
    properties.SetPropertyBool("ignoreMaterials",
                               m_program.get<bool>("--ignore-materials"));
    properties.SetPropertyBool("generateLODs",
                               m_program.get<bool>("--generate-lods"));

    if (exporter.Export(&scene, "sne", p.string(), 0, &properties) !=
        AI_SUCCESS) {
      return std::unexpected{exporter.GetErrorString()};
    }
    return stopwatch.count();
  }

private:
  argparse::ArgumentParser m_program;
  Assimp::Importer m_importer;
  std::unique_ptr<CustomLogger> m_logger;
};

} // namespace

int main(int argc, char *argv[]) {
#ifdef _DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif

  return App{}.execute(argc, argv);
}
