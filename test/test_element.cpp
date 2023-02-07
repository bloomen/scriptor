#include "helper.h"

#include "../src/element.h"

TEST(element, from_json_with_all_tags)
{
    const std::string json =
        R"({"c":"doner","s":123124141241,"l":2,"p":456,"t":123,"f":"file.txt","i":99,"n":"foo","m":"blah"})";
    const scriptor::Element exp{
        spdlog::log_clock::time_point{std::chrono::seconds{123124141241}},
        spdlog::level::info,
        "[doner] [456:123] [file.txt:99:foo] blah"};
    const auto res = scriptor::Element::from_json(json);
    ASSERT_EQ(exp.time, res.time);
    ASSERT_EQ(exp.level, res.level);
    ASSERT_EQ(exp.message, res.message);
}

TEST(element, from_json_with_all_tags_and_all_string)
{
    const std::string json =
        R"({"c":"doner","s":"123124141241","l":"2","p":"456","t":"123","f":"file.txt","i":99,"n":"foo","m":"blah"})";
    const scriptor::Element exp{
        spdlog::log_clock::time_point{std::chrono::seconds{123124141241}},
        spdlog::level::info,
        "[doner] [456:123] [file.txt:99:foo] blah"};
    const auto res = scriptor::Element::from_json(json);
    ASSERT_EQ(exp.time, res.time);
    ASSERT_EQ(exp.level, res.level);
    ASSERT_EQ(exp.message, res.message);
}

TEST(element, from_json_with_none)
{
    const std::string json = "{}";
    const scriptor::Element exp{
        spdlog::log_clock::now(), spdlog::level::trace, "[] [:] [::] "};
    const auto res = scriptor::Element::from_json(json);
    ASSERT_EQ(exp.level, res.level);
    ASSERT_EQ(exp.message, res.message);
}

TEST(element, from_json_with_fractional_time)
{
    const std::string json = R"({"c":"doner","s":12345.567,"m":"blah"})";
    spdlog::log_clock::time_point time{std::chrono::seconds{12345}};
    time += std::chrono::microseconds{567000};
    const scriptor::Element exp{
        time, spdlog::level::trace, "[doner] [:] [::] blah"};
    const auto res = scriptor::Element::from_json(json);
    ASSERT_EQ(exp.level, res.level);
    ASSERT_EQ(exp.message, res.message);
}

TEST(element, from_json_with_fractional_time2)
{
    const std::string json = R"({"c":"doner","s":12345.5678901,"m":"blah"})";
    spdlog::log_clock::time_point time{std::chrono::seconds{12345}};
    time += std::chrono::microseconds{567890};
    const scriptor::Element exp{
        time, spdlog::level::trace, "[doner] [:] [::] blah"};
    const auto res = scriptor::Element::from_json(json);
    ASSERT_EQ(exp.level, res.level);
    ASSERT_EQ(exp.message, res.message);
}

TEST(element, from_json_with_broken_json)
{
    const std::string json = R"({"c":"doner","s":12345.5678901,"m:"blah"})";
    ASSERT_ANY_THROW(scriptor::Element::from_json(json));
}

TEST(element, from_json_with_broken_time)
{
    const std::string json = R"({"c":"doner","s":not_a_time,"m":"blah"})";
    ASSERT_ANY_THROW(scriptor::Element::from_json(json));
}

TEST(element, from_json_with_broken_level)
{
    const std::string json = R"({"c":"doner","l":not_a_level,"m":"blah"})";
    ASSERT_ANY_THROW(scriptor::Element::from_json(json));
}
