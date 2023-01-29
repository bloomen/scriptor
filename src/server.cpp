#include <filesystem>
#include <iostream>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/systemd_sink.h>
#include <spdlog/spdlog.h>

#include "server.h"

namespace scriptor
{

Server::Server(const std::size_t n_threads, const std::string& file)
    : m_acceptor{m_ioc, aio::local::stream_protocol::endpoint{file}}
{
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

    spdlog::flush_on(spdlog::level::warn);

    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "/tmp/mylog.txt", 1024 * 1024 * 10, 3);
    rotating_sink->set_level(spdlog::level::debug);
    rotating_sink->set_pattern("[%Y-%m-%dT%H:%M:%S.%f] [%l] %v");

    auto systemd_sink =
        std::make_shared<spdlog::sinks::systemd_sink_mt>("scriptor");
    systemd_sink->set_level(spdlog::level::debug);

    std::initializer_list<spdlog::sink_ptr> sinks{rotating_sink, systemd_sink};
    m_logger = std::make_shared<spdlog::logger>("scriptor", sinks);
    m_logger->set_level(spdlog::level::debug);

    m_log_thread = std::thread{[this] { worker(); }};
}

Server::~Server()
{
    m_logger->flush();
    shutdown();
    {
        std::lock_guard lock(m_mutex);
        m_done = true;
    }
    m_cv.notify_one();
    m_log_thread.join();
}

void
Server::accept()
{
    m_acceptor.async_accept([this](
                                const boost::system::error_code ec,
                                aio::local::stream_protocol::socket&& socket) {
        if (!ec)
        {
            std::make_shared<Session>(std::move(socket), [this](Element&& e) {
                std::lock_guard lock{m_mutex};
                m_queue.emplace(std::move(e));
                m_cv.notify_one();
            })->read();
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

                std::string prefix = "[" + e.channel + "] ";

                prefix += "[" + std::to_string(e.process_id) + ":" +
                    std::to_string(e.thread_id) + "] ";

                std::string location;
                if (e.filename)
                {
                    location += *e.filename;
                    if (e.line)
                    {
                        location += ":" + std::to_string(*e.line);
                    }
                    if (e.func)
                    {
                        location += ":";
                    }
                }
                if (e.func)
                {
                    location += *e.func;
                }

                if (!location.empty())
                {
                    prefix += "[" + location + "] ";
                }

                m_logger->log(e.time,
                              spdlog::source_loc{},
                              e.log_level,
                              prefix + e.message);

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
