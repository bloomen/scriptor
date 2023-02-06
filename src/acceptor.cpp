#include "acceptor.h"

namespace scriptor
{

UnixAcceptor::UnixAcceptor(asio::io_context& ioc,
                           asio::local::stream_protocol::endpoint ep)
    : m_acceptor{ioc, std::move(ep)}
{
}

void
UnixAcceptor::async_accept(
    std::function<void(asio::error_code, std::unique_ptr<Socket>)> cb)
{
    m_acceptor.async_accept(
        [cb](const auto ec, asio::local::stream_protocol::socket&& socket) {
            cb(ec, std::make_unique<UnixSocket>(std::move(socket)));
        });
}

TcpAcceptor::TcpAcceptor(asio::io_context& ioc, asio::ip::tcp::endpoint ep)
    : m_acceptor{ioc, std::move(ep)}
{
}

void
TcpAcceptor::async_accept(
    std::function<void(asio::error_code, std::unique_ptr<Socket>)> cb)
{
    m_acceptor.async_accept(
        [cb](const auto ec, asio::ip::tcp::socket&& socket) {
            cb(ec, std::make_unique<TcpSocket>(std::move(socket)));
        });
}

std::unique_ptr<Acceptor>
make_acceptor(asio::io_context& ioc,
              const std::string& file,
              const std::string& ip,
              const asio::ip::port_type port)
{
    if (!file.empty())
    {
        return std::make_unique<UnixAcceptor>(
            ioc, asio::local::stream_protocol::endpoint{file});
    }
    if (!ip.empty())
    {
        const auto address = asio::ip::address::from_string(ip);
        return std::make_unique<TcpAcceptor>(
            ioc, asio::ip::tcp::endpoint{address, port});
    }
    throw std::runtime_error{
        "Must provide either socket_file or socket_address!"};
    return nullptr;
}

} // namespace scriptor
