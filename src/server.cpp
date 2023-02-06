#include <filesystem>
#include <iostream>

#include <fmt/core.h>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#ifdef SCRIPTOR_LINUX
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/systemd_sink.h>
#endif
#include <spdlog/spdlog.h>

#include "server.h"

namespace scriptor
{

Server::Server(const Options& opt)
    : m_ioc{static_cast<int>(opt.n_threads)}
{
    m_acceptor = make_acceptor(
        m_ioc, opt.socket_file, opt.socket_address, opt.socket_port);

    // Setup loggers
    const std::string pattern = "[%Y-%m-%dT%H:%M:%S.%f] [%l] %v";

    if (!opt.file_sink_filename.empty())
    {
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(
            opt.file_sink_filename,
            opt.file_sink_max_file_size,
            opt.file_sink_max_files);
        file_sink->set_level(
            static_cast<spdlog::level::level_enum>(opt.file_sink_log_level));
        file_sink->set_pattern(pattern);
        m_loggers.emplace_back(std::move(file_sink));
    }

#ifdef SCRIPTOR_LINUX
    if (opt.systemd_sink_use)
    {
        auto systemd_sink = std::make_shared<spdlog::sinks::systemd_sink_st>(
            opt.identity, false);
        systemd_sink->set_level(
            static_cast<spdlog::level::level_enum>(opt.systemd_sink_log_level));
        m_loggers.emplace_back(std::move(systemd_sink));
    }

    if (opt.syslog_sink_use)
    {
        auto syslog_sink = std::make_shared<spdlog::sinks::syslog_sink_st>(
            opt.identity, LOG_PID, LOG_USER, false);
        syslog_sink->set_level(
            static_cast<spdlog::level::level_enum>(opt.syslog_sink_log_level));
        m_loggers.emplace_back(std::move(syslog_sink));
    }
#endif

    if (m_loggers.empty())
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern(pattern);
        m_loggers.emplace_back(std::move(console_sink));
    }

    // Setup producers
    accept();
    while (m_ioc_threads.size() < opt.n_threads)
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
            m_ioc_threads.push_back(std::move(thread));
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
    // Shutdown producers
    shutdown();

    // Shutdown consumers
    m_loggers.clear();
}

void
Server::accept()
{
    m_acceptor->async_accept(
        [this](const auto ec, std::unique_ptr<Socket>&& socket) {
            if (!ec)
            {
                std::make_shared<Session>(
                    std::move(socket),
                    [this](std::vector<Element>&& elements) {
                        if (m_loggers.size() == 1)
                        {
                            m_loggers.front().push(std::move(elements));
                        }
                        else
                        {
                            for (auto& logger : m_loggers)
                            {
                                logger.push(elements);
                            }
                        }
                    })
                    ->read();
            }
            accept();
        });
}

void
Server::shutdown()
{
    m_ioc.stop();
    for (auto& t : m_ioc_threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

} // namespace scriptor
