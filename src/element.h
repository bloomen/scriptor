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
    std::string process_id; // p
    std::string thread_id; // t
    std::string filename; // f
    std::string line; // i
    std::string func; // n

    static Element
    from_xml(const std::string& xml);
};

} // namespace scriptor
