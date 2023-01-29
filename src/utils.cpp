#include <tuple>
#include <vector>

#include "utils.h"

namespace scriptor
{

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

} // namespace scriptor
