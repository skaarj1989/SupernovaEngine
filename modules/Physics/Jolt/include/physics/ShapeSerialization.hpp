#pragma once

#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"

#include <filesystem>
#include <expected>

[[nodiscard]] bool saveShape(const std::filesystem::path &,
                             const JPH::ShapeSettings::ShapeResult &);
[[nodiscard]] std::expected<JPH::Shape::ShapeResult, std::string>
loadShape(const std::filesystem::path &);
