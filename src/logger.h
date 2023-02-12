#include <condition_variable>
#include <mutex>
#include <thread>

#include <spdlog/logger.h>
#include <spdlog/sinks/sink.h>

#include "element.h"

namespace scriptor
{

class Logger
{
public:
    explicit Logger(spdlog::sink_ptr&& sink);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger&
    operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger&
    operator=(Logger&&) = delete;

    void
    push(const std::vector<Element>& elements);
    void
    push(std::vector<Element>&& elements);

private:
    void
    worker();

    spdlog::sink_ptr m_sink;
    bool m_done{false};
    std::thread m_thread;
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::vector<Element> m_elements;
};

} // namespace scriptor
