#pragma once

#include <string>

struct NameComponent {
  std::string name;

  template <class Archive> void serialize(Archive &archive) { archive(name); }
};

static_assert(std::is_copy_constructible_v<NameComponent>);
