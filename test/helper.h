#pragma once

#include <filesystem>
#include <fstream>
#include <random>
#include <thread>

#include <fmt/core.h>

#if __clang__ || __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <gtest/gtest.h>
#if __clang__ || __GNUC__
#pragma GCC diagnostic pop
#endif

#include "../src/aio.h"

void
set_utc();

std::string
random_string(std::string::size_type length);

std::string
read_from_file(const std::string& filename);
