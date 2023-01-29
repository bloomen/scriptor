#include <filesystem>

#include "server.h"

namespace scriptor
{

Server::Server(const std::size_t n_threads, const std::string& file)
    : m_acceptor{m_ioc, aio::local::stream_protocol::endpoint{file}}
{
    accept();

    while (m_threads.size() < n_threads)
    {
        std::thread thread;
        try
        {
            thread = std::thread{[this] { m_ioc.run(); }};
        }
        catch (...)
        {
            shutdown();
            throw;
        }
        try
        {
            m_threads.push_back(std::move(thread));
        }
        catch (...)
        {
            shutdown();
            thread.join();
            throw;
        }
    }
}

Server::~Server()
{
    shutdown();
}

void
Server::accept()
{
    m_acceptor.async_accept(
        [this](const boost::system::error_code ec,
               aio::local::stream_protocol::socket&& socket) {
            if (!ec)
            {
                std::make_shared<Session>(std::move(socket))->read();
            }
            accept();
        });
}

void
Server::shutdown()
{
    for (auto& t : m_threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

} // namespace scriptor
