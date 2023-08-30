#pragma once

#include "assimp/scene.h"
#include <iosfwd>

std::ostream &operator<<(std::ostream &os, const aiScene &);
