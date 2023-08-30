#pragma once

#include "assimp/DefaultLogger.hpp"
#include <format>

#define LOG(level, msg) Assimp::DefaultLogger::get()->level(msg)
#define LOG_EX(level, fmt, ...) LOG(level, std::format(fmt, __VA_ARGS__))
