#include <future>

#include "helper.h"

#include "../src/logger.h"

namespace
{

class MockSink : public spdlog::sinks::sink
{
public:
    explicit MockSink(const std::size_t count)
        : count{count}
        , promise{}
        , future{promise.get_future()}
    {
    }

    void
    wait()
    {
        future.wait();
    }

    void
    log(const spdlog::details::log_msg& msg) override
    {
        std::lock_guard lock{mutex};
        std::string message{msg.payload.begin(), msg.payload.end()};
        elements.push_back({msg.time, msg.level, std::move(message)});
    }

    void
    flush() override
    {
        std::lock_guard lock{mutex};
        ++flush_count;
        if (flush_count == count)
        {
            promise.set_value();
        }
    }

    void
    set_pattern(const std::string&) override
    {
        throw std::runtime_error{"not implemented"};
    }

    void set_formatter(std::unique_ptr<spdlog::formatter>) override
    {
        throw std::runtime_error{"not implemented"};
    }

    std::vector<scriptor::Element>
    get_elements() const
    {
        std::lock_guard lock{mutex};
        return elements;
    }

    std::size_t
    get_flush_count() const
    {
        std::lock_guard lock{mutex};
        return flush_count;
    }

private:
    std::size_t count;
    std::promise<void> promise;
    std::future<void> future;
    mutable std::mutex mutex;
    std::vector<scriptor::Element> elements;
    std::size_t flush_count = 0;
};

} // namespace

TEST(logger, sink_receives_two_elements)
{
    const std::vector<scriptor::Element> elements{
        scriptor::Element{
            spdlog::log_clock::time_point{std::chrono::seconds{123124141241}},
            spdlog::level::warn,
            "[doner] [456:123] [file.txt:99:foo] blah"},
        scriptor::Element{
            spdlog::log_clock::now(), spdlog::level::err, "other message"},
    };
    auto sink = std::make_shared<MockSink>(elements.size());
    scriptor::Logger logger{sink};
    logger.push(elements);
    sink->wait();
    ASSERT_EQ(elements, sink->get_elements());
    ASSERT_EQ(2u, sink->get_flush_count());
}

TEST(logger, sink_receives_two_elements_rvalue)
{
    std::vector<scriptor::Element> elements{
        scriptor::Element{
            spdlog::log_clock::time_point{std::chrono::seconds{123124141241}},
            spdlog::level::info,
            "[doner] [456:123] [file.txt:99:foo] blah"},
        scriptor::Element{
            spdlog::log_clock::now(), spdlog::level::warn, "other message"},
    };
    const auto expected = elements;
    auto sink = std::make_shared<MockSink>(1);
    scriptor::Logger logger{sink};
    logger.push(std::move(elements));
    sink->wait();
    ASSERT_EQ(expected, sink->get_elements());
    ASSERT_EQ(1u, sink->get_flush_count());
}

TEST(logger, sink_receives_six_elements)
{
    const std::vector<scriptor::Element> elements{
        scriptor::Element{
            spdlog::log_clock::time_point{std::chrono::seconds{123124141241}},
            spdlog::level::info,
            "[doner] [456:123] [file.txt:99:foo] blah"},
        scriptor::Element{
            spdlog::log_clock::now(), spdlog::level::warn, "other message"},
    };
    auto sink = std::make_shared<MockSink>(3);
    scriptor::Logger logger{sink};
    logger.push(elements);
    logger.push(elements);
    logger.push(elements);
    sink->wait();
    const std::vector<scriptor::Element> expected{
        elements[0],
        elements[1],
        elements[0],
        elements[1],
        elements[0],
        elements[1],
    };
    ASSERT_EQ(expected, sink->get_elements());
    ASSERT_EQ(3u, sink->get_flush_count());
}
