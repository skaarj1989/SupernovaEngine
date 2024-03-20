#include "ProjectCommands.hpp"
#include "os/FileSystem.hpp"
#include "MaterialEditor/MaterialProject.hpp"
#include "renderer/WorldRenderer.hpp"

#include "spdlog/logger.h"
#include "spdlog/fmt/chrono.h"

namespace {

constexpr auto kFallbackDomain = gfx::MaterialDomain::Surface;

using clock = std::chrono::steady_clock;

}; // namespace

//
// NewMaterialCommand class:
//

NewMaterialCommand::NewMaterialCommand(MaterialProject &project,
                                       const gfx::MaterialDomain domain)
    : m_project{&project}, m_domain{domain} {}

bool NewMaterialCommand::execute(const Context &ctx) {
  ctx.logger.trace(__FUNCTION__);

  m_project->init(m_domain);
  ctx.logger.info("Material initialized (domain={}).", gfx::toString(m_domain));
  return true;
}

//
// LoadProjectCommand class:
//

LoadProjectCommand::LoadProjectCommand(MaterialProject &project,
                                       const std::filesystem::path &p)
    : m_project{&project}, m_path{p} {}

bool LoadProjectCommand::execute(const Context &ctx) {
  ctx.logger.trace(__FUNCTION__);

  const auto relativePath =
    os::FileSystem::relativeToRoot(m_path)->generic_string();
  if (auto error = m_project->load(m_path); !error) {
    ctx.logger.info("Material project loaded from: '{}'.", relativePath);
    return true;
  } else {
    ctx.logger.error("Failed to load MaterialProject from: '{}' [{}]",
                     relativePath, *error);
    return NewMaterialCommand{*m_project, kFallbackDomain}.execute(ctx);
  }
}

//
// SaveProjectCommand class:
//

SaveProjectCommand::SaveProjectCommand(MaterialProject &project,
                                       const std::filesystem::path &p)
    : m_project{&project}, m_path{p} {
  assert(!m_path.empty());
}

bool SaveProjectCommand::execute(const Context &ctx) {
  ctx.logger.trace(__FUNCTION__);

  const auto relativePath =
    os::FileSystem::relativeToRoot(m_path)->generic_string();
  if (auto error = m_project->saveAs(m_path); !error) {
    ctx.logger.info("MaterialProject saved to: '{}'.", relativePath);
    return true;
  } else {
    ctx.logger.error("Failed to save MaterialProject to: '{}'. {}",
                     relativePath, *error);
    return false;
  }
}

//
// ExportMaterialCommand class:
//

ExportMaterialCommand::ExportMaterialCommand(const MaterialProject &project,
                                             const std::filesystem::path &p)
    : m_project{&project}, m_path{p} {
  assert(!m_path.empty());
}

bool ExportMaterialCommand::execute(const Context &ctx) {
  ctx.logger.trace(__FUNCTION__);

  const auto relativePath =
    os::FileSystem::relativeToRoot(m_path.parent_path())->generic_string();
  if (auto error = m_project->exportMaterial(m_path); !error) {
    ctx.logger.info("Material exported to: '{}'.", relativePath);
    return true;
  } else {
    ctx.logger.error("Failed to export material to: '{}'. {}", relativePath,
                     *error);
    return false;
  }
}

//
// ComposeShaderCommand class:
//

ComposeShaderCommand::ComposeShaderCommand(MaterialProject &project,
                                           const rhi::ShaderStages stages)
    : m_project{&project}, m_stages{stages} {
  assert(m_stages != rhi::ShaderStages{});
}

bool ComposeShaderCommand::execute(const Context &ctx) {
  ctx.logger.trace(__FUNCTION__);

  const auto begin = clock::now();
  if (auto errorMessage = m_project->compose(m_stages); !errorMessage) {
    ctx.logger.info("Shader(s) composed in: {}.",
                    std::chrono::duration_cast<std::chrono::microseconds>(
                      clock::now() - begin));
    return true;
  } else {
    ctx.logger.error(*errorMessage);
    return false;
  }
}

//
// BuildMaterialCommand class:
//

BuildMaterialCommand::BuildMaterialCommand(MaterialProject &project,
                                           const gfx::WorldRenderer &renderer)
    : m_project{&project}, m_renderer{&renderer} {}

bool BuildMaterialCommand::execute(const Context &ctx) {
  ctx.logger.trace(__FUNCTION__);

  const auto begin = clock::now();
  if (m_project->buildMaterial(*m_renderer)) {
    ctx.logger.info("Material built in: {}.",
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                      clock::now() - begin));
    return true;
  } else {
    ctx.logger.error("Corrupted material.");
    return false;
  }
}
