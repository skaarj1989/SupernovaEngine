#pragma once

#include "MaterialEditor/Command.hpp"
#include "renderer/Material.hpp"

namespace gfx {
class WorldRenderer;
};

class MaterialProject;

class NewMaterialCommand : public Command {
public:
  NewMaterialCommand(MaterialProject &, const gfx::MaterialDomain);

  bool execute(const Context &) override;

private:
  MaterialProject *m_project;
  gfx::MaterialDomain m_domain;
};

class LoadProjectCommand : public Command {
public:
  LoadProjectCommand(MaterialProject &, const std::filesystem::path &);

  bool execute(const Context &) override;

private:
  MaterialProject *m_project;
  std::filesystem::path m_path;
};
class SaveProjectCommand : public Command {
public:
  SaveProjectCommand(MaterialProject &, const std::filesystem::path &);

  bool execute(const Context &) override;

private:
  MaterialProject *m_project;
  std::filesystem::path m_path;
};
class ExportMaterialCommand : public Command {
public:
  ExportMaterialCommand(const MaterialProject &, const std::filesystem::path &);

  bool execute(const Context &) override;

private:
  const MaterialProject *m_project;
  std::filesystem::path m_path;
};

class ComposeShaderCommand : public Command {
public:
  ComposeShaderCommand(MaterialProject &, const rhi::ShaderStages);

  bool execute(const Context &) override;

private:
  MaterialProject *m_project;
  rhi::ShaderStages m_stages;
};

class BuildMaterialCommand : public Command {
public:
  BuildMaterialCommand(MaterialProject &, const gfx::WorldRenderer &);

  bool execute(const Context &) override;

private:
  MaterialProject *m_project;
  const gfx::WorldRenderer *m_renderer;
};
