#include <stdlib.h>
#include <time.h>

#include "helper.h"

void
set_utc()
{
#ifdef SCRIPTOR_WINDOWS
    // TODO find the equivalent for Windows
#else
    setenv("TZ", "UTC", 1);
    tzset();
#endif
}

std::string
random_string(std::string::size_type length)
{
    static auto& chrs = "0123456789"
                        "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type>
        pick(0, sizeof(chrs) - 2);

    std::string s;
    s.reserve(length);
    while (length--)
    {
        s += chrs[pick(rg)];
    }
    return s;
}

std::string
read_from_file(const std::string& filename)
{
    std::ifstream is{filename};
    return std::string{std::istreambuf_iterator<char>{is},
                       std::istreambuf_iterator<char>{}};
}
