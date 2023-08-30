#pragma once

#include "assimp/scene.h"
#include "assimp/Exporter.hpp"

void supernovaMeshExportFunction(const char *filePath, Assimp::IOSystem *,
                                 const aiScene *,
                                 const Assimp::ExportProperties *);
