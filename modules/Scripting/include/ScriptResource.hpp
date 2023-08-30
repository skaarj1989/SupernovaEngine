#pragma once

#include "Resource.hpp"

class ScriptResource final : public Resource {
public:
  explicit ScriptResource(std::string s, const std::filesystem::path &p)
      : Resource{p}, code{std::move(s)} {}

  std::string code;
};
