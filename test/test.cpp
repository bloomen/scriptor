#if __clang__ || __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <gtest/gtest.h>
#if __clang__ || __GNUC__
#pragma GCC diagnostic pop
#endif

#include "element.h"
#include "session.h"

TEST(element, from_xml_with_all_tags)
{
    const std::string xml = "<c>doner</c><t>123</t><l>2</l><p>456</p><s>123124141241</s><f>file.txt</f><i>99</i><n>foo()</n><m>blah</m>";
    scriptor::Element e;
    e.channel = "doner";
    e.time = spdlog::log_clock::time_point{std::chrono::seconds{123124141241}};
    e.level = spdlog::level::info;
    e.message = "blah";
    e.process = "456";
    e.thread = "123";
    e.filename = "file.txt";
    e.line = "99";
    e.func = "foo()";
    const auto res = scriptor::Element::from_xml(xml);
    ASSERT_EQ(e, res);
}

TEST(element, from_xml_with_only_required)
{
    const std::string xml = "<c>doner</c><m>blah</m>";
    scriptor::Element e;
    e.channel = "doner";
    e.message = "blah";
    const auto res = scriptor::Element::from_xml(xml);
    ASSERT_EQ(e, res);
}

TEST(element, from_xml_with_escaped_xml)
{
    const std::string xml = "<c>do&amp;ner</c><m>bl&lt;ah</m>";
    scriptor::Element e;
    e.channel = "do&ner";
    e.message = "bl<ah";
    const auto res = scriptor::Element::from_xml(xml);
    ASSERT_EQ(e, res);
}

TEST(element, from_xml_with_broken_xml)
{
    const std::string xml = "<c>doner</c><t<m>blah</m>";
    ASSERT_ANY_THROW(scriptor::Element::from_xml(xml));
}

TEST(element, from_xml_with_broken_time)
{
    const std::string xml = "<c>doner</c><s>not_a_time</s><m>blah</m>";
    ASSERT_ANY_THROW(scriptor::Element::from_xml(xml));
}

TEST(element, from_xml_with_broken_level)
{
    const std::string xml = "<c>doner</c><l>not_a_level</l><m>blah</m>";
    ASSERT_ANY_THROW(scriptor::Element::from_xml(xml));
}

TEST(processor, one_shot)
{
    const std::string data = "<c>doner</c><m>blah</m>";
    scriptor::Element e;
    e.channel = "doner";
    e.message = "blah";
    scriptor::Processor proc;
    const auto res = proc(data.data(), data.size());
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ(e, res[0]);
}

TEST(processor, one_shot_and_again)
{
    const std::string data1 = "<c>doner</c><m>blah</m>";
    scriptor::Element e1;
    e1.channel = "doner";
    e1.message = "blah";

    scriptor::Processor proc;
    const auto res1 = proc(data1.data(), data1.size());
    ASSERT_EQ(1u, res1.size());
    ASSERT_EQ(e1, res1[0]);

    const std::string data2 = "<c>bar</c><m>foo</m>";
    scriptor::Element e2;
    e2.channel = "bar";
    e2.message = "foo";

    const auto res2 = proc(data2.data(), data2.size());
    ASSERT_EQ(1u, res2.size());
    ASSERT_EQ(e2, res2[0]);
}

TEST(processor, one_shot_with_two_elements)
{
    const std::string data = "asfa<c>doner</c><m>guanchen</m>wfqfgvwev<c>bar</c><m>dani</m>wfqwf";
    scriptor::Element e1;
    e1.channel = "doner";
    e1.message = "guanchen";
    scriptor::Element e2;
    e2.channel = "bar";
    e2.message = "dani";
    scriptor::Processor proc;
    const auto res = proc(data.data(), data.size());
    ASSERT_EQ(2u, res.size());
    ASSERT_EQ(e1, res[0]);
    ASSERT_EQ(e2, res[1]);
}

TEST(processor, one_shot_with_invalid_xml)
{
    const std::string data = "<c>doner</c><t<m>blah</m>";
    scriptor::Processor proc;
    const auto res = proc(data.data(), data.size());
    ASSERT_EQ(0u, res.size());
}

TEST(processor, two_shots)
{
    const std::string data1 = "<c>doner</c><m>bl";
    const std::string data2 = "ah</m>";
    scriptor::Element e;
    e.channel = "doner";
    e.message = "blah";
    scriptor::Processor proc;
    const auto res1 = proc(data1.data(), data1.size());
    ASSERT_EQ(0u, res1.size());
    const auto res2 = proc(data2.data(), data2.size());
    ASSERT_EQ(1u, res2.size());
    ASSERT_EQ(e, res2[0]);
}

TEST(processor, two_shots_with_crap_at_either_end)
{
    const std::string data1 = "abc<c>doner</c><m>bl";
    const std::string data2 = "ah</m>dfg";
    scriptor::Element e;
    e.channel = "doner";
    e.message = "blah";
    scriptor::Processor proc;
    const auto res1 = proc(data1.data(), data1.size());
    ASSERT_EQ(0u, res1.size());
    const auto res2 = proc(data2.data(), data2.size());
    ASSERT_EQ(1u, res2.size());
    ASSERT_EQ(e, res2[0]);
}

TEST(processor, two_shots_with_weird_tags)
{
    const std::string data1 = "abc</m><c>doner</c><m>bl";
    const std::string data2 = "ah</m><c>dfg";
    scriptor::Element e;
    e.channel = "doner";
    e.message = "blah";
    scriptor::Processor proc;
    const auto res1 = proc(data1.data(), data1.size());
    ASSERT_EQ(0u, res1.size());
    const auto res2 = proc(data2.data(), data2.size());
    ASSERT_EQ(1u, res2.size());
    ASSERT_EQ(e, res2[0]);
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
