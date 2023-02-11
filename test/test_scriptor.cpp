#include "helper.h"

#include "../src/scriptor.h"

namespace
{

class CoutRedirect
{
public:
    explicit CoutRedirect(std::streambuf* const new_buffer)
        : m_buffer(std::cout.rdbuf(new_buffer))
    {
    }

    ~CoutRedirect()
    {
        std::cout.rdbuf(m_buffer);
    }

private:
    std::streambuf* m_buffer;
};

} // namespace

TEST(scriptor, get_help)
{
    std::vector<char*> argv{
        (char*)"scriptor",
        (char*)"--help",
    };
    std::stringstream buffer;
    CoutRedirect cr{buffer.rdbuf()};
    const auto code = scriptor::run(static_cast<int>(argv.size()), argv.data());
    ASSERT_EQ(0, code);
    const std::string text = buffer.str();
    ASSERT_NE(text.find("Display this help message"), std::string::npos);
}

TEST(scriptor, get_help_with_short_arg)
{
    std::vector<char*> argv{
        (char*)"scriptor",
        (char*)"-h",
    };
    std::stringstream buffer;
    CoutRedirect cr{buffer.rdbuf()};
    const auto code = scriptor::run(static_cast<int>(argv.size()), argv.data());
    ASSERT_EQ(0, code);
    const std::string text = buffer.str();
    ASSERT_NE(text.find("Display this help message"), std::string::npos);
}

void
test_scriptor_run(bool tcp)
{
    const std::string message1 =
        R"({"c":"doner","s":"123124141241","l":"2","p":"456","t":"123","f":"file.txt","i":99,"n":"foo","m":"blah"})";
    const std::string message2 =
        R"({"c":"analysis","s":1675153178.487972,"l":3,"p":5887,"f":"client.py","i":48,"n":"compute","m":"hello there"})";
    const auto socket_file = random_string(10);
    const asio::ip::port_type port = 12346;
    const std::string port_str = std::to_string(port);
    const auto filelog_filename = random_string(10);
    std::vector<char*> argv{(char*)"scriptor",
                            (char*)"--filelog_filename",
                            (char*)filelog_filename.c_str()};
    if (tcp)
    {
        argv.push_back((char*)"--socket_address");
        argv.push_back((char*)"127.0.0.1");
        argv.push_back((char*)"--socket_port");
        argv.push_back((char*)port_str.c_str());
    }
    else
    {
        argv.push_back((char*)"--socket_file");
        argv.push_back((char*)socket_file.c_str());
    }
    std::thread thread{[tcp, &socket_file, port, &message1, &message2] {
        std::this_thread::sleep_for(std::chrono::seconds{1});
        if (tcp)
        {
            send_to_tcp_socket(port, message1);
            std::this_thread::sleep_for(std::chrono::milliseconds{200});
            send_to_tcp_socket(port, message2);
        }
        else
        {
            send_to_unix_socket(socket_file, message1);
            std::this_thread::sleep_for(std::chrono::milliseconds{200});
            send_to_unix_socket(socket_file, message2);
        }
        std::this_thread::sleep_for(std::chrono::seconds{1});
        scriptor::stop(SIGINT);
    }};
    const auto code = scriptor::run(static_cast<int>(argv.size()), argv.data());
    ASSERT_EQ(0, code);
    thread.join();
    if (!tcp)
    {
        std::filesystem::remove(socket_file);
    }
    const std::string exp =
        R"([1779-10-09T12:57:50.18446744073708584755] [info] [doner] [456:123] [file.txt:99:foo] blah
[2023-01-31T21:19:38.487972] [warning] [analysis] [5887:] [client.py:48:compute] hello there
)";
    const auto content = read_from_file(filelog_filename);
    ASSERT_EQ(exp, content);
    std::filesystem::remove(filelog_filename);
}

TEST(scriptor, run_using_unix_socket)
{
    test_scriptor_run(false);
}

TEST(scriptor, run_using_tcp_socket)
{
    test_scriptor_run(true);
}
