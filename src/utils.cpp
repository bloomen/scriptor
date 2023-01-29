#include <tuple>
#include <vector>

#include <boost/algorithm/string/replace.hpp>

#include "utils.h"

namespace scriptor
{

void
xml_unescape(std::string& xml)
{
    static const std::vector<std::pair<std::string, std::string>> seq{
        {"&amp;", "&"},
        {"&quot;", "\""},
        {"&apos;", "'"},
        {"&lt;", "<"},
        {"&gt;", ">"},
    };

    if (xml.find_first_of('&') == std::string::npos)
    {
        return;
    }

    for (const auto& p : seq)
    {
        boost::replace_all(xml, p.first, p.second);
    }
}

} // namespace scriptor
