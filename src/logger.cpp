#include <iostream>

#include "logger.h"

namespace scriptor
{

Logger::Logger(spdlog::sink_ptr sink)
    : m_sink{std::move(sink)}
{
    m_thread = std::thread{[this] { worker(); }};
}

Logger::~Logger()
{
    {
        std::lock_guard lock(m_mutex);
        m_done = true;
    }
    m_cv.notify_one();
    m_thread.join();
}

void
Logger::push(const std::vector<Element>& elements)
{
    std::lock_guard lock{m_mutex};
    m_elements.reserve(m_elements.size() + elements.size());
    m_elements.insert(m_elements.end(), elements.begin(), elements.end());
    m_cv.notify_one();
}

void
Logger::push(std::vector<Element>&& elements)
{
    std::lock_guard lock{m_mutex};
    m_elements.reserve(m_elements.size() + elements.size());
    for (auto&& e : elements)
    {
        m_elements.emplace_back(std::move(e));
    }
    m_cv.notify_one();
}

void
Logger::worker()
{
    for (;;)
    {
        std::vector<Element> elements;
        {
            std::unique_lock lock(m_mutex);
            m_cv.wait(lock, [this] { return m_done || !m_elements.empty(); });
            if (m_done)
            {
                m_sink->flush();
                break;
            }
            std::swap(elements, m_elements);
        }
        try
        {
            std::size_t count = 0;
            for (const auto& e : elements)
            {
                if (!m_sink->should_log(e.level))
                {
                    continue;
                }
                const spdlog::details::log_msg msg{e.time,
                                                   spdlog::source_loc{},
                                                   spdlog::string_view_t{},
                                                   e.level,
                                                   e.message};
                m_sink->log(msg);
                ++count;
            }
            if (count > 0)
            {
                m_sink->flush();
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Logger Error: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Logger Error: Unknown" << std::endl;
        }
    }
}

} // namespace scriptor
