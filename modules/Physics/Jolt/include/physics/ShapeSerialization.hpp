#pragma once

#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"

#include <expected>
#include <filesystem>

[[nodiscard]] bool saveShape(const std::filesystem::path &,
                             const JPH::ShapeSettings::ShapeResult &);
[[nodiscard]] std::expected<JPH::Shape::ShapeResult, std::string>
loadShape(const std::filesystem::path &);
