#pragma once

#include <spdlog/common.h>

#include <optional>
#include <string>

namespace scriptor
{

void
xml_unescape(std::string& xml);

struct Element
{
    std::string channel; // c
    std::optional<spdlog::log_clock::time_point> time; // s
    spdlog::level::level_enum level = spdlog::level::trace; // l
    std::string message; // m
    std::string process; // p
    std::string thread; // t
    std::string filename; // f
    std::string line; // i
    std::string func; // n

    bool
    operator==(const Element& o) const;

    auto
    make_tie() const
    {
        return std::tie(channel,
                        time,
                        level,
                        message,
                        process,
                        thread,
                        filename,
                        line,
                        func);
    }

    static Element
    from_xml(const std::string& xml);
};

} // namespace scriptor
