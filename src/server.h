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
    std::string socket_file;
    std::string identity = "scriptor";
    std::size_t n_threads = std::thread::hardware_concurrency();
    std::string file_sink_filename;
    std::size_t file_sink_max_file_size = 1024 * 1024 * 10;
    std::size_t file_sink_max_files = 3;
    spdlog::level::level_enum file_sink_log_level = spdlog::level::trace;
    bool systemd_sink_use = false;
    spdlog::level::level_enum systemd_sink_log_level = spdlog::level::trace;
    bool syslog_sink_use = false;
    spdlog::level::level_enum syslog_sink_log_level = spdlog::level::trace;
};

class Server
{
public:
    explicit Server(const Options& opt);
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
