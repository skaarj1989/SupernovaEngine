#pragma once

#include "entt/entity/registry.hpp"
#include "entt/entity/snapshot.hpp"

#define CEREAL_FUTURE_EXPERIMENTAL
#include "cereal/archives/adapters.hpp"

#include <utility>

using InputContext = std::pair<entt::registry &, entt::snapshot_loader &>;
using OutputContext = std::pair<const entt::registry &, const entt::snapshot &>;
