#pragma once

#include <spdlog/common.h>

#include <optional>
#include <string>

namespace scriptor
{

struct Element
{
    std::string channel; // c
    spdlog::log_clock::time_point time; // u
    spdlog::level::level_enum log_level = spdlog::level::debug; // l
    std::string message; // m
    int process_id = 0; // p
    int thread_id = 0; // t
    std::optional<std::string> filename; // f
    std::optional<int> line; // i
    std::optional<std::string> func; // n

    static Element
    from_xml(const std::string& xml);
};

} // namespace scriptor
