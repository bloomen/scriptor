#pragma once

#include <spdlog/common.h>

#include <string>

namespace scriptor
{

void
xml_unescape(std::string& xml);

struct Element
{
    spdlog::log_clock::time_point time;
    spdlog::level::level_enum level;
    std::string message;

    static Element
    from_xml(const std::string& xml);
};

} // namespace scriptor
