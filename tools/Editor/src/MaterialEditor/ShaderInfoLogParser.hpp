#pragma once

#include <map>
#include <string>

using ErrorMarkers = std::map<int32_t, std::string>;

[[nodiscard]] ErrorMarkers getErrorMarkers(const std::string_view);
