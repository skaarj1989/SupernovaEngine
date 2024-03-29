#pragma once

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

#include <expected>
#include <filesystem>

[[nodiscard]] bool saveShape(const std::filesystem::path &,
                             const JPH::ShapeSettings::ShapeResult &);
[[nodiscard]] std::expected<JPH::Shape::ShapeResult, std::string>
loadShape(const std::filesystem::path &);
