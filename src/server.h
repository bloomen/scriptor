#pragma once

#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>

#include <spdlog/logger.h>

#include "session.h"

namespace scriptor
{

struct Options
{
};

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
    worker();

    void
    shutdown();

    aio::io_context m_ioc;
    std::vector<std::thread> m_ioc_threads;
    aio::local::stream_protocol::acceptor m_acceptor;
    bool m_done{false};
    std::thread m_log_thread;
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::queue<Element> m_queue;
    std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace scriptor
