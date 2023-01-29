#include <chrono>

#include <boost/property_tree/detail/file_parser_error.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "element.h"

namespace pt = boost::property_tree;

namespace scriptor
{

Element
Element::from_xml(const std::string& xml)
{
    pt::ptree tree;
    std::stringstream ss;
    ss << xml;
    pt::read_xml(ss, tree);

    Element e;

    // channel
    e.channel = tree.get_child("c").get_value<std::string>();

    // time
    if (auto time_us = tree.get_child_optional("u"))
    {
        const std::chrono::microseconds us{time_us->get_value<std::uint64_t>()};
        e.time = spdlog::log_clock::time_point{us};
    }
    else
    {
        e.time = spdlog::log_clock::now();
    }

    // log level
    if (auto log_level = tree.get_child_optional("l"))
    {
        const auto level = log_level->get_value<int>();
        if (level >= spdlog::level::trace && level < spdlog::level::n_levels)
        {
            e.log_level = static_cast<spdlog::level::level_enum>(level);
        }
    }

    // message
    e.message = tree.get_child("m").get_value<std::string>();

    // process id
    if (auto process_id = tree.get_child_optional("p"))
    {
        e.process_id = process_id->get_value<std::string>();
    }

    // thread id
    if (auto thread_id = tree.get_child_optional("t"))
    {
        e.thread_id = thread_id->get_value<std::string>();
    }

    // filename
    if (auto filename = tree.get_child_optional("f"))
    {
        e.filename = filename->get_value<std::string>();
    }

    // line
    if (auto line = tree.get_child_optional("i"))
    {
        e.line = line->get_value<std::string>();
    }

    // func
    if (auto func = tree.get_child_optional("n"))
    {
        e.func = func->get_value<std::string>();
    }

    return e;
}

} // namespace scriptor
