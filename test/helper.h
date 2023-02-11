#pragma once

#include <filesystem>
#include <fstream>
#include <random>

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
send_to_unix_socket(const std::string& socket_file, const std::string& message);

void
send_to_tcp_socket(const asio::ip::port_type port, const std::string& message);

std::string
random_string(std::string::size_type length);

std::string
read_from_file(const std::string& filename);
