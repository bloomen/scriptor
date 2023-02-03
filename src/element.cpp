#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <fmt/core.h>

#include "element.h"

namespace scriptor
{

class XmlError : public std::exception
{
public:
    const char*
    what() const noexcept override
    {
        return "XML Error";
    }
};

void
xml_unescape(std::string& xml)
{
    static const std::vector<std::pair<std::string, char>> seq{
        {"&amp;", '&'},
        {"&quot;", '\"'},
        {"&apos;", '\''},
        {"&lt;", '<'},
        {"&gt;", '>'},
    };

    auto i = xml.find('&');
    if (i == std::string::npos)
    {
        return;
    }

    std::string res;
    res.reserve(xml.size());
    std::size_t prev = 0;
    while (i != std::string::npos)
    {
        res.append(&xml[prev], i - prev);
        prev = i;
        for (const auto& p : seq)
        {
            if ((xml.size() - i) >= p.first.size() &&
                std::equal(&xml[i], &xml[i] + p.first.size(), p.first.begin()))
            {
                res.push_back(p.second);
                prev += p.first.size();
                break;
            }
        }
        if (prev == i)
        {
            res.push_back('&');
            ++prev;
        }
        i = xml.find('&', prev);
    }
    if (prev < xml.size())
    {
        res.append(&xml[prev], xml.size() - prev);
    }
    xml = std::move(res);
}

bool
Element::operator==(const Element& o) const
{
    return make_tie() == o.make_tie();
}

Element
Element::from_xml(const std::string& xml)
{
    Element e;
    std::unordered_map<char, std::string> values;

    auto cur = xml.find('<');
    while (cur != std::string::npos && cur + 7 < xml.size())
    {
        const auto tag = xml[cur + 1];
        if (xml[cur + 2] != '>')
        {
            break;
        }
        const auto start = cur + 3;
        const auto token = fmt::format("</{}>", tag);
        const auto end = xml.find('<', start);
        if (end == std::string::npos)
        {
            break;
        }
        if (end + token.size() > xml.size())
        {
            break;
        }
        if (!std::equal(&xml[end], &xml[end] + token.size(), token.begin()))
        {
            break;
        }
        values[tag] = xml.substr(start, end - start);
        cur = xml.find('<', end + token.size());
    }

    auto c = values.find('c');
    if (c != values.end())
    {
        e.channel = std::move(c->second);
        xml_unescape(e.channel);
    }
    else
    {
        throw XmlError{};
    }

    auto s = values.find('s');
    if (s != values.end())
    {
        const std::chrono::microseconds us{
            static_cast<std::uint64_t>(std::stod(s->second) * 1e6)};
        e.time = spdlog::log_clock::time_point{us};
    }

    auto l = values.find('l');
    if (l != values.end())
    {
        const auto lev = std::stoi(l->second);
        if (lev >= spdlog::level::trace && lev < spdlog::level::n_levels)
        {
            e.level = static_cast<spdlog::level::level_enum>(lev);
        }
    }

    auto m = values.find('m');
    if (m != values.end())
    {
        e.message = std::move(m->second);
        xml_unescape(e.message);
    }
    else
    {
        throw XmlError{};
    }

    auto p = values.find('p');
    if (p != values.end())
    {
        e.process = std::move(p->second);
        xml_unescape(e.process);
    }

    auto t = values.find('t');
    if (t != values.end())
    {
        e.thread = std::move(t->second);
        xml_unescape(e.thread);
    }

    auto f = values.find('f');
    if (f != values.end())
    {
        e.filename = std::move(f->second);
        xml_unescape(e.filename);
    }

    auto i = values.find('i');
    if (i != values.end())
    {
        e.line = std::move(i->second);
        xml_unescape(e.line);
    }

    auto n = values.find('n');
    if (n != values.end())
    {
        e.func = std::move(n->second);
        xml_unescape(e.func);
    }

    return e;
}

} // namespace scriptor
