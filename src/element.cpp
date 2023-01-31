#include <boost/property_tree/xml_parser.hpp>

#include "element.h"

namespace scriptor
{

bool
Element::operator==(const Element& o) const
{
    return make_tie() == o.make_tie();
}

Element
Element::from_xml(const std::string& xml)
{
    boost::property_tree::ptree tree;
    std::stringstream ss;
    ss << xml;
    boost::property_tree::read_xml(ss, tree);

    Element e;

    // channel
    e.channel = tree.get_child("c").get_value<std::string>();

    // time
    if (auto time = tree.get_child_optional("s"))
    {
        const std::chrono::microseconds ns{
            static_cast<std::uint64_t>(time->get_value<double>() * 1e6)};
        e.time = spdlog::log_clock::time_point{ns};
    }

    // level
    if (auto level = tree.get_child_optional("l"))
    {
        const auto lev = level->get_value<int>();
        if (lev >= spdlog::level::trace && lev < spdlog::level::n_levels)
        {
            e.level = static_cast<spdlog::level::level_enum>(lev);
        }
    }

    // message
    e.message = tree.get_child("m").get_value<std::string>();

    // process
    if (auto process = tree.get_child_optional("p"))
    {
        e.process = process->get_value<std::string>();
    }

    // thread
    if (auto thread = tree.get_child_optional("t"))
    {
        e.thread = thread->get_value<std::string>();
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
