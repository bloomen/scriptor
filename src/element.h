#pragma once

#include <spdlog/common.h>

#include <optional>
#include <string>

namespace scriptor
{

struct Element
{
    std::string channel; // c
    std::optional<spdlog::log_clock::time_point> time; // u
    spdlog::level::level_enum log_level = spdlog::level::debug; // l
    std::string message; // m
    std::string process_id; // p
    std::string thread_id; // t
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
                        log_level,
                        message,
                        process_id,
                        thread_id,
                        filename,
                        line,
                        func);
    }

    static Element
    from_xml(const std::string& xml);
};

} // namespace scriptor
