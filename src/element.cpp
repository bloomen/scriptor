#include <nlohmann/json.hpp>

#include <fmt/core.h>

#include "element.h"

using json = nlohmann::json;

namespace scriptor
{

Element
Element::from_json(const std::string& str)
{
    const auto data = json::parse(str);

    auto find_string = [&data](const auto token) -> std::string {
        auto it = data.find(token);
        if (it != data.end())
        {
            if (it.value().is_null())
            {
                return {};
            }
            if (it.value().is_number())
            {
                const auto num = it.value().template get<std::int64_t>();
                return std::to_string(num);
            }
            return it.value().template get<std::string>();
        }
        return {};
    };

    spdlog::log_clock::time_point time;
    {
        auto s = data.find("s");
        if (s != data.end() && !s.value().is_null())
        {
            double value;
            if (s.value().is_string())
            {
                value = std::stod(s.value().get<std::string>());
            }
            else
            {
                value = s.value().get<double>();
            }
            const auto duration =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::duration<double>{value});
            time = spdlog::log_clock::time_point{duration};
        }
        else
        {
            time = spdlog::log_clock::now();
        }
    }

    spdlog::level::level_enum level = spdlog::level::trace;
    {
        auto l = data.find("l");
        if (l != data.end() && !l.value().is_null())
        {
            int value;
            if (l.value().is_string())
            {
                value = std::stoi(l.value().get<std::string>());
            }
            else
            {
                value = l.value().get<int>();
            }
            if (value >= spdlog::level::trace &&
                value < spdlog::level::n_levels)
            {
                level = static_cast<spdlog::level::level_enum>(value);
            }
        }
    }

    auto message = fmt::format("[{0}] [{1}:{2}] [{3}:{4}:{5}] {6}",
                               find_string("c"),
                               find_string("p"),
                               find_string("t"),
                               find_string("f"),
                               find_string("i"),
                               find_string("n"),
                               find_string("m"));

    return {std::move(time), level, std::move(message)};
}

} // namespace scriptor
