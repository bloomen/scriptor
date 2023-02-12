#include "socket.h"

namespace scriptor
{

UnixSocket::UnixSocket(asio::local::stream_protocol::socket&& socket)
    : m_socket{std::move(socket)}
{
}

void
UnixSocket::async_read_some(
    asio::mutable_buffers_1&& buffer,
    std::function<void(asio::error_code, std::size_t)>&& cb)
{
    m_socket.async_read_some(std::move(buffer), std::move(cb));
}

TcpSocket::TcpSocket(asio::ip::tcp::socket&& socket)
    : m_socket{std::move(socket)}
{
}

void
TcpSocket::async_read_some(
    asio::mutable_buffers_1&& buffer,
    std::function<void(asio::error_code, std::size_t)>&& cb)
{
    m_socket.async_read_some(std::move(buffer), std::move(cb));
}

} // namespace scriptor
