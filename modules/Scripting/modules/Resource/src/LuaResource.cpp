#include "LuaResource.hpp"
#include "sol/state.hpp"

#include "Resource.hpp"
#include "Sol2HelperMacros.hpp"

#include <format>

void registerResource(sol::state &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(Resource,
    sol::no_constructor,

    "valid", &Resource::operator bool,

    _BIND(Resource, getResourceId),
    "getPath", [](const Resource &self) { return self.getPath().string(); },

    sol::meta_function::equal_to,
      sol::overload(
        [](const Resource &self, const Resource &other) { return self == other; },
        [](const Resource &self, const std::string_view other) { return self == other; }
      ),

    sol::meta_function::to_string,
      [](const Resource &self) {
        return self ? std::format("Resource({})", self.getResourceId())
                    : "InvalidResource";
      }
  );
  // clang-format on
}
