#include "helper.h"

void
send_to_unix_socket(const std::string& socket_file, const std::string& message)
{
    asio::io_context ioc;
    asio::local::stream_protocol::socket socket{ioc};
    const asio::local::stream_protocol::endpoint endpoint{socket_file};
    socket.connect(endpoint);
    asio::write(socket, asio::buffer(message));
}

void
send_to_tcp_socket(const asio::ip::port_type port, const std::string& message)
{
    asio::io_context ioc;
    asio::ip::tcp::socket socket{ioc};
    const asio::ip::tcp::endpoint endpoint{
        asio::ip::address::from_string("127.0.0.1"), port};
    socket.connect(endpoint);
    asio::write(socket, asio::buffer(message));
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
