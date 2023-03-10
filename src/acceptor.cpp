#include "acceptor.h"

namespace scriptor
{

UnixAcceptor::UnixAcceptor(asio::io_context& ioc,
                           asio::local::stream_protocol::endpoint&& ep)
    : m_acceptor{ioc, std::move(ep)}
{
}

void
UnixAcceptor::async_accept(
    std::function<void(asio::error_code, std::unique_ptr<Socket>)>&& cb)
{
    m_acceptor.async_accept(
        [cb = std::move(cb)](const auto ec,
                             asio::local::stream_protocol::socket&& socket) {
            cb(ec, std::make_unique<UnixSocket>(std::move(socket)));
        });
}

TcpAcceptor::TcpAcceptor(asio::io_context& ioc, asio::ip::tcp::endpoint&& ep)
    : m_acceptor{ioc, std::move(ep)}
{
}

void
TcpAcceptor::async_accept(
    std::function<void(asio::error_code, std::unique_ptr<Socket>)>&& cb)
{
    m_acceptor.async_accept(
        [cb = std::move(cb)](const auto ec, asio::ip::tcp::socket&& socket) {
            cb(ec, std::make_unique<TcpSocket>(std::move(socket)));
        });
}

} // namespace scriptor
