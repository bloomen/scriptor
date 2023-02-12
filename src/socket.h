#pragma once
#include <functional>

#include "aio.h"

namespace scriptor
{

class Socket
{
public:
    virtual ~Socket() = default;

    virtual void
    async_read_some(
        asio::mutable_buffers_1&& buffer,
        std::function<void(asio::error_code, std::size_t)>&& cb) = 0;
};

class UnixSocket : public Socket
{
public:
    explicit UnixSocket(asio::local::stream_protocol::socket&& socket);

    void
    async_read_some(
        asio::mutable_buffers_1&& buffer,
        std::function<void(asio::error_code, std::size_t)>&& cb) override;

private:
    asio::local::stream_protocol::socket m_socket;
};

class TcpSocket : public Socket
{
public:
    explicit TcpSocket(asio::ip::tcp::socket&& socket);

    void
    async_read_some(
        asio::mutable_buffers_1&& buffer,
        std::function<void(asio::error_code, std::size_t)>&& cb) override;

private:
    asio::ip::tcp::socket m_socket;
};

} // namespace scriptor
