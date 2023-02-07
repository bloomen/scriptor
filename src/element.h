#pragma once

#include <spdlog/common.h>

#include <string>

namespace scriptor
{

struct Element
{
    spdlog::log_clock::time_point time;
    spdlog::level::level_enum level;
    std::string message;

    static Element
    from_json(const std::string& str);

    bool
    operator==(const Element& o) const;
};

} // namespace scriptor
