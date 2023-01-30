#if __clang__ || __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <gtest/gtest.h>
#if __clang__ || __GNUC__
#pragma GCC diagnostic pop
#endif

#include "element.h"
#include "utils.h"

TEST(utils, xml_unescape_with_nothing_to_escape)
{
    std::string xml = "<a>albert einstein</a>";
    const auto exp = xml;
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(utils, xml_unescape_with_amp)
{
    std::string xml = "<a>albert&amp;einstein</a>";
    scriptor::xml_unescape(xml);
    const std::string exp = "<a>albert&einstein</a>";
    ASSERT_EQ(exp, xml);
}

TEST(utils, xml_unescape_with_amp_at_beginning)
{
    std::string xml = "&amp;albert einstein</a>";
    const std::string exp = "&albert einstein</a>";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(utils, xml_unescape_with_amp_at_end)
{
    std::string xml = "albert einstein&amp;";
    const std::string exp = "albert einstein&";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(utils, xml_unescape_with_all_escapers)
{
    std::string xml = "&quot;alb&lt;ert&apos;eins&gt;tein&amp;";
    const std::string exp = "\"alb<ert'eins>tein&";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(utils, xml_unescape_with_only_escapers)
{
    std::string xml = "&quot;&lt;&lt;&gt;&apos;&amp;&gt;&apos;&amp;";
    const std::string exp = "\"<<>'&>'&";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(utils, xml_unescape_with_escapers_and_random_ampersands)
{
    std::string xml = "&&quot;alb&lt;er&t&apos;eins&gt;&tein&amp;&";
    const std::string exp = "&\"alb<er&t'eins>&tein&&";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(utils, xml_unescape_with_half_escapers)
{
    std::string xml = "quot;alb&lt;ert&aos;eins&gt;tein&amp";
    const std::string exp = "quot;alb<ert&aos;eins>tein&amp";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(element, from_xml_with_all_tags)
{
    const std::string xml = "<c>doner</c><t>123</t><l>2</l><p>456</p><u>123124141241</u><f>file.txt</f><i>99</i><n>foo()</n><m>blah</m>";
    scriptor::Element e;
    e.channel = "doner";
    e.time = spdlog::log_clock::time_point{std::chrono::microseconds{123124141241}};
    e.log_level = spdlog::level::info;
    e.message = "blah";
    e.process_id = "456";
    e.thread_id = "123";
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

TEST(element, from_xml_with_broken_xml)
{
    const std::string xml = "<c>doner</c><t<m>blah</m>";
    ASSERT_ANY_THROW(scriptor::Element::from_xml(xml));
}

TEST(element, from_xml_with_broken_time)
{
    const std::string xml = "<c>doner</c><u>not_a_time</u><m>blah</m>";
    ASSERT_ANY_THROW(scriptor::Element::from_xml(xml));
}

TEST(element, from_xml_with_broken_level)
{
    const std::string xml = "<c>doner</c><l>not_a_level</l><m>blah</m>";
    ASSERT_ANY_THROW(scriptor::Element::from_xml(xml));
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
