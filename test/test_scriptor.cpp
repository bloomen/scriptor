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

class CerrRedirect
{
public:
    explicit CerrRedirect(std::streambuf* const new_buffer)
        : m_buffer(std::cerr.rdbuf(new_buffer))
    {
    }

    ~CerrRedirect()
    {
        std::cerr.rdbuf(m_buffer);
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

TEST(scriptor, with_no_options)
{
    std::vector<char*> argv{
        (char*)"scriptor",
    };
    std::stringstream buffer;
    CerrRedirect cr{buffer.rdbuf()};
    const auto code = scriptor::run(static_cast<int>(argv.size()), argv.data());
    ASSERT_EQ(1, code);
}

TEST(scriptor, with_invalid_option)
{
    std::vector<char*> argv{(char*)"scriptor",
                            (char*)"--socket_address",
                            (char*)"some_file",
                            (char*)"--socket_port",
                            (char*)"invalid_port"};
    std::stringstream buffer;
    CerrRedirect cr{buffer.rdbuf()};
    const auto code = scriptor::run(static_cast<int>(argv.size()), argv.data());
    ASSERT_EQ(1, code);
}

void
test_scriptor_run(const bool tcp, const bool unix, const int log_level)
{
    asio::io_context ioc;
    const std::string message1 =
        R"({"c":"doner","s":"1253370013","l":"2","p":"456","t":"123","f":"file.txt","i":99,"n":"foo","m":"blah"}\n)";
    const std::string message2 =
        R"({"c":"analysis","s":1675153178.487972,"l":3,"p":5887,"f":"client.py","i":48,"n":"compute","m":"hello there"}\n)";
    const auto socket_file = random_string(10);
    const asio::ip::port_type port = 12346;
    const std::string port_str = std::to_string(port);
    const auto filelog_filename = random_string(10);
    const auto log_level_str = std::to_string(log_level);
    std::vector<char*> argv{(char*)"scriptor",
                            (char*)"--filelog_filename",
                            (char*)filelog_filename.c_str(),
                            (char*)"--filelog_level",
                            (char*)log_level_str.c_str()};
    if (tcp)
    {
        argv.push_back((char*)"--socket_address");
        argv.push_back((char*)"127.0.0.1");
        argv.push_back((char*)"--socket_port");
        argv.push_back((char*)port_str.c_str());
    }
    if (unix)
    {
        argv.push_back((char*)"--socket_file");
        argv.push_back((char*)socket_file.c_str());
    }
    std::thread thread{
        [&ioc, tcp, unix, &socket_file, port, &message1, &message2] {
            std::this_thread::sleep_for(std::chrono::seconds{1});
            if (tcp)
            {
                asio::ip::tcp::socket socket{ioc};
                const asio::ip::tcp::endpoint endpoint{
                    asio::ip::address::from_string("127.0.0.1"), port};
                socket.connect(endpoint);
                socket.send(asio::buffer(message1), 0);
                std::this_thread::sleep_for(std::chrono::milliseconds{500});
                socket.send(asio::buffer(message2), 0);
                std::this_thread::sleep_for(std::chrono::milliseconds{500});
            }
            if (unix)
            {
                asio::local::stream_protocol::socket socket{ioc};
                const asio::local::stream_protocol::endpoint endpoint{
                    socket_file};
                socket.connect(endpoint);
                socket.send(asio::buffer(message1), 0);
                std::this_thread::sleep_for(std::chrono::milliseconds{500});
                socket.send(asio::buffer(message2), 0);
                std::this_thread::sleep_for(std::chrono::milliseconds{500});
            }
            std::this_thread::sleep_for(std::chrono::seconds{1});
            scriptor::stop(SIGINT);
        }};
    const auto code = scriptor::run(static_cast<int>(argv.size()), argv.data());
    thread.join();
    if (!tcp && !unix)
    {
        ASSERT_EQ(1, code);
        return;
    }
    else
    {
        ASSERT_EQ(0, code);
    }
    if (unix)
    {
        std::filesystem::remove(socket_file);
    }
    std::string exp;
    if (log_level <= 2)
    {
        exp =
            R"([2009-09-19T14:20:13.000000] [info] [doner] [456:123] [file.txt:99:foo] blah
[2023-01-31T08:19:38.487972] [warning] [analysis] [5887:] [client.py:48:compute] hello there
)";
    }
    else
    {
        exp =
            R"([2023-01-31T08:19:38.487972] [warning] [analysis] [5887:] [client.py:48:compute] hello there
)";
    }
    if (tcp && unix)
    {
        exp += exp;
    }
    const auto content = read_from_file(filelog_filename);
    ASSERT_EQ(exp, content);
    std::filesystem::remove(filelog_filename);
}

#ifndef SCRIPTOR_WINDOWS
TEST(scriptor, run_using_unix_socket_at_info_level)
{
    test_scriptor_run(false, true, 2);
}

TEST(scriptor, run_using_unix_socket_at_warning_level)
{
    test_scriptor_run(false, true, 3);
}

TEST(scriptor, run_using_tcp_and_unix_socket_at_info_level)
{
    test_scriptor_run(true, true, 2);
}

TEST(scriptor, run_using_tcp_and_unix_socket_at_warning_level)
{
    test_scriptor_run(true, true, 3);
}
#endif

TEST(scriptor, run_using_tcp_socket_at_info_level)
{
    test_scriptor_run(true, false, 2);
}

TEST(scriptor, run_using_tcp_socket_at_warning_level)
{
    test_scriptor_run(true, false, 3);
}

TEST(scriptor, run_using_neither_socket)
{
    std::stringstream buffer;
    CerrRedirect cr{buffer.rdbuf()};
    test_scriptor_run(false, false, 2);
}
