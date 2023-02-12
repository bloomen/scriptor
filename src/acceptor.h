#pragma once
#include <memory>

#include "socket.h"

namespace scriptor
{

class Acceptor
{
public:
    virtual ~Acceptor() = default;

    virtual void
    async_accept(std::function<void(asio::error_code,
                                    std::unique_ptr<Socket>)>&& cb) = 0;
};

class UnixAcceptor : public Acceptor
{
public:
    UnixAcceptor(asio::io_context& ioc,
                 asio::local::stream_protocol::endpoint&& ep);

    void
    async_accept(std::function<void(asio::error_code,
                                    std::unique_ptr<Socket>)>&& cb) override;

private:
    asio::local::stream_protocol::acceptor m_acceptor;
};

class TcpAcceptor : public Acceptor
{
public:
    TcpAcceptor(asio::io_context& ioc, asio::ip::tcp::endpoint&& ep);

    void
    async_accept(std::function<void(asio::error_code,
                                    std::unique_ptr<Socket>)>&& cb) override;

private:
    asio::ip::tcp::acceptor m_acceptor;
};

std::unique_ptr<Acceptor>
make_acceptor(asio::io_context& ioc,
              const std::string& file,
              const std::string& ip,
              asio::ip::port_type port);

} // namespace scriptor
