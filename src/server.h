#pragma once

#include <list>

#include <spdlog/logger.h>

#include "acceptor.h"
#include "logger.h"
#include "session.h"

namespace scriptor
{

struct Options
{
    std::string socket_file;
    std::string socket_address;
    asio::ip::port_type socket_port = 12345;
    std::string identity = "scriptor";
    std::size_t n_threads = std::thread::hardware_concurrency();
    std::string file_sink_filename;
    std::size_t file_sink_max_file_size = 1024 * 1024 * 10;
    std::size_t file_sink_max_files = 3;
    int file_sink_log_level = spdlog::level::trace;
    bool systemd_sink_use = false;
    int systemd_sink_log_level = spdlog::level::trace;
    bool syslog_sink_use = false;
    int syslog_sink_log_level = spdlog::level::trace;
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
    shutdown();

    asio::io_context m_ioc;
    std::unique_ptr<Acceptor> m_acceptor;
    std::vector<std::thread> m_ioc_threads;
    std::list<Logger> m_loggers;
};

} // namespace scriptor
