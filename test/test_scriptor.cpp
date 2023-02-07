#include "helper.h"

#include "../src/scriptor.h"

TEST(scriptor, get_help)
{
    std::vector<char*> argv{
        (char*)"scriptor",
        (char*)"--help",
    };
    std::stringstream buffer;
    std::cout.rdbuf(buffer.rdbuf());
    const auto code = scriptor::run(static_cast<int>(argv.size()), argv.data());
    ASSERT_EQ(0, code);
    const std::string text = buffer.str();
    ASSERT_NE(text.find("Display this help message"), std::string::npos);
}

// TEST(scriptor, run_using_unix_socket)
//{
//    std::vector<char*> argv{
//        (char*)"scriptor",
//        (char*)"--socket_file",
//        (char*)"/tmp/scriptor.sock",
//    };
//    scriptor::run(static_cast<int>(argv.size()), argv.data());
//}
