#include "helper.h"

#include "../src/session.h"

TEST(processor, one_shot)
{
    const std::string data = R"({"c":"doner","m":"blah"})";
    const scriptor::Element exp{spdlog::log_clock::now(),
                                spdlog::level::trace,
                                "[doner] [:] [::] blah"};
    scriptor::Processor proc;
    const auto res = proc(data.data(), data.size());
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ(exp.level, res[0].level);
    ASSERT_EQ(exp.message, res[0].message);
}

TEST(processor, one_shot_and_again)
{
    const std::string data1 = R"({"c":"doner","m":"blah"})";
    const scriptor::Element exp1{spdlog::log_clock::now(),
                                 spdlog::level::trace,
                                 "[doner] [:] [::] blah"};

    scriptor::Processor proc;
    const auto res1 = proc(data1.data(), data1.size());
    ASSERT_EQ(1u, res1.size());
    ASSERT_EQ(exp1.level, res1[0].level);
    ASSERT_EQ(exp1.message, res1[0].message);

    const std::string data2 = R"({"c":"bar","m":"foo"})";
    const scriptor::Element exp2{
        spdlog::log_clock::now(), spdlog::level::trace, "[bar] [:] [::] foo"};

    const auto res2 = proc(data2.data(), data2.size());
    ASSERT_EQ(1u, res2.size());
    ASSERT_EQ(exp2.level, res2[0].level);
    ASSERT_EQ(exp2.message, res2[0].message);
}

TEST(processor, one_shot_with_two_elements)
{
    const std::string data =
        R"(asfa{"c":"doner","m":"guanchen"}wfqfgvwev{"c":"bar","m":"dani"}wfqwf)";
    const scriptor::Element exp1{spdlog::log_clock::now(),
                                 spdlog::level::trace,
                                 "[doner] [:] [::] guanchen"};
    const scriptor::Element exp2{
        spdlog::log_clock::now(), spdlog::level::trace, "[bar] [:] [::] dani"};
    scriptor::Processor proc;
    const auto res = proc(data.data(), data.size());
    ASSERT_EQ(2u, res.size());
    ASSERT_EQ(exp1.level, res[0].level);
    ASSERT_EQ(exp1.message, res[0].message);
    ASSERT_EQ(exp2.level, res[1].level);
    ASSERT_EQ(exp2.message, res[1].message);
}

TEST(processor, one_shot_with_invalid_json)
{
    const std::string data = R"({"c":"bar","m:"foo"})";
    scriptor::Processor proc;
    const auto res = proc(data.data(), data.size());
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ(spdlog::level::critical, res[0].level);
}

TEST(processor, two_shots)
{
    const std::string data1 = R"({"c":"doner","m":"bl)";
    const std::string data2 = R"(ah"})";
    const scriptor::Element exp{spdlog::log_clock::now(),
                                spdlog::level::trace,
                                "[doner] [:] [::] blah"};
    scriptor::Processor proc;
    const auto res1 = proc(data1.data(), data1.size());
    ASSERT_EQ(0u, res1.size());
    const auto res2 = proc(data2.data(), data2.size());
    ASSERT_EQ(1u, res2.size());
    ASSERT_EQ(exp.level, res2[0].level);
    ASSERT_EQ(exp.message, res2[0].message);
}

TEST(processor, two_shots_with_crap_at_either_end)
{
    const std::string data1 = R"(abc{"c":"doner","m":"bl)";
    const std::string data2 = R"(ah"}dfg)";
    const scriptor::Element exp{spdlog::log_clock::now(),
                                spdlog::level::trace,
                                "[doner] [:] [::] blah"};
    scriptor::Processor proc;
    const auto res1 = proc(data1.data(), data1.size());
    ASSERT_EQ(0u, res1.size());
    const auto res2 = proc(data2.data(), data2.size());
    ASSERT_EQ(1u, res2.size());
    ASSERT_EQ(exp.level, res2[0].level);
    ASSERT_EQ(exp.message, res2[0].message);
}
