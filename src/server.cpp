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
    , m_acceptor{m_ioc, aio::local::stream_protocol::endpoint{opt.socket_file}}
{
    // 1. Instantiate logging objects
    spdlog::flush_on(spdlog::level::warn);
    const std::string pattern = "[%Y-%m-%dT%H:%M:%S.%f] [%l] %v";

    std::vector<spdlog::sink_ptr> sinks;

    if (!opt.file_sink_filename.empty())
    {
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            opt.file_sink_filename,
            opt.file_sink_max_file_size,
            opt.file_sink_max_files);
        file_sink->set_level(
            static_cast<spdlog::level::level_enum>(opt.file_sink_log_level));
        file_sink->set_pattern(pattern);
        sinks.push_back(std::move(file_sink));
    }

#ifdef SCRIPTOR_LINUX
    if (opt.systemd_sink_use)
    {
        auto systemd_sink = std::make_shared<spdlog::sinks::systemd_sink_mt>(
            opt.identity, false);
        systemd_sink->set_level(
            static_cast<spdlog::level::level_enum>(opt.systemd_sink_log_level));
        sinks.push_back(std::move(systemd_sink));
    }

    if (opt.syslog_sink_use)
    {
        auto syslog_sink = std::make_shared<spdlog::sinks::syslog_sink_mt>(
            opt.identity, LOG_PID, LOG_USER, false);
        syslog_sink->set_level(
            static_cast<spdlog::level::level_enum>(opt.syslog_sink_log_level));
        sinks.push_back(std::move(syslog_sink));
    }
#endif

    if (sinks.empty())
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern(pattern);
        sinks.push_back(std::move(console_sink));
    }

    m_logger = std::make_shared<spdlog::logger>(
        "scriptor", sinks.begin(), sinks.end());
    m_logger->set_level(spdlog::level::trace);

    // 2. Setup consumer thread
    m_log_thread = std::thread{[this] { worker(); }};

    // 3. Setup producers
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
    // 3. Shutdown producers
    shutdown();

    // 2. Shutdown consumer
    {
        std::lock_guard lock(m_mutex);
        m_done = true;
    }
    m_cv.notify_one();
    m_log_thread.join();

    // 1. Flush logger
    m_logger->flush();
}

void
Server::accept()
{
    m_acceptor.async_accept([this](
                                const boost::system::error_code ec,
                                aio::local::stream_protocol::socket&& socket) {
        if (!ec)
        {
            std::make_shared<Session>(std::move(socket),
                                      [this](std::vector<Element>&& elements) {
                                          std::lock_guard lock{m_mutex};
                                          for (auto&& e : elements)
                                          {
                                              m_queue.emplace(std::move(e));
                                          }
                                          m_cv.notify_one();
                                      })
                ->read();
        }
        accept();
    });
}

void
Server::worker()
{
    for (;;)
    {
        std::queue<Element> elements;
        {
            std::unique_lock lock(m_mutex);
            m_cv.wait(lock, [this] { return m_done || !m_queue.empty(); });
            if (m_done)
            {
                break;
            }
            std::swap(elements, m_queue);
        }
        try
        {
            while (!elements.empty())
            {
                const auto& e = elements.front();

                const auto message =
                    fmt::format("[{0}] [{1}:{2}] [{3}:{4}:{5}] {6}",
                                e.channel,
                                e.process,
                                e.thread,
                                e.filename,
                                e.line,
                                e.func,
                                e.message);

                const auto time = e.time ? *e.time : spdlog::log_clock::now();

                m_logger->log(time, spdlog::source_loc{}, e.level, message);

                elements.pop();
            }
            m_logger->flush();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Server Error: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Server Error: Unknown" << std::endl;
        }
    }
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
