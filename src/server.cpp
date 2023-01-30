#include <filesystem>
#include <iostream>

#include <fmt/core.h>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/systemd_sink.h>
#include <spdlog/spdlog.h>

#include "server.h"

namespace scriptor
{

Server::Server(const std::size_t n_threads, const std::string& file)
    : m_acceptor{m_ioc, aio::local::stream_protocol::endpoint{file}}
{
    // 1. Instantiate logging objects
    const std::string ident = "FOOBAR";

    spdlog::flush_on(spdlog::level::warn);

    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "/tmp/mylog.txt", 1024 * 1024 * 10, 3);
    rotating_sink->set_level(spdlog::level::trace);
    rotating_sink->set_pattern("[%Y-%m-%dT%H:%M:%S.%f] [%l] %v");

    auto systemd_sink =
        std::make_shared<spdlog::sinks::systemd_sink_mt>(ident, false);
    systemd_sink->set_level(spdlog::level::trace);

    auto syslog_sink = std::make_shared<spdlog::sinks::syslog_sink_mt>(
        ident, LOG_PID, LOG_USER, false);
    syslog_sink->set_level(spdlog::level::trace);

    std::initializer_list<spdlog::sink_ptr> sinks{
        rotating_sink, systemd_sink, syslog_sink};
    m_logger = std::make_shared<spdlog::logger>("scriptor", sinks);
    m_logger->set_level(spdlog::level::trace);

    // 2. Setup consumer thread
    m_log_thread = std::thread{[this] { worker(); }};

    // 3. Setup producers
    accept();
    while (m_ioc_threads.size() < n_threads)
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
                                e.process_id,
                                e.thread_id,
                                e.filename,
                                e.line,
                                e.func,
                                e.message);

                const auto time = e.time ? *e.time : spdlog::log_clock::now();

                m_logger->log(time, spdlog::source_loc{}, e.log_level, message);

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
