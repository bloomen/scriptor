#pragma once

#include <vector>

#include "session.h"

namespace scriptor
{

class Server
{
public:
    Server(std::size_t n_threads, const std::string& file);
    ~Server();

    Server(const Server&) = delete;
    Server&
    operator=(const Server&) = delete;
    Server(Server&&) = delete;
    Server&
    operator=(Server&&) = delete;

private:
    void
    accept();

    void
    shutdown();

    aio::io_context m_ioc;
    std::vector<std::thread> m_threads;
    aio::local::stream_protocol::acceptor m_acceptor;
};

} // namespace scriptor
