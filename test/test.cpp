#if __clang__ || __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <gtest/gtest.h>
#if __clang__ || __GNUC__
#pragma GCC diagnostic pop
#endif

#include "utils.h"

TEST(scriptor, xml_unescape_with_nothing_to_escape)
{
    std::string xml = "<a>albert einstein</a>";
    const auto exp = xml;
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(scriptor, xml_unescape_with_amp)
{
    std::string xml = "<a>albert&amp;einstein</a>";
    scriptor::xml_unescape(xml);
    const std::string exp = "<a>albert&einstein</a>";
    ASSERT_EQ(exp, xml);
}

TEST(scriptor, xml_unescape_with_amp_at_beginning)
{
    std::string xml = "&amp;albert einstein</a>";
    const std::string exp = "&albert einstein</a>";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(scriptor, xml_unescape_with_amp_at_end)
{
    std::string xml = "albert einstein&amp;";
    const std::string exp = "albert einstein&";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(scriptor, xml_unescape_with_all_escapers)
{
    std::string xml = "&quot;alb&lt;ert&apos;eins&gt;tein&amp;";
    const std::string exp = "\"alb<ert'eins>tein&";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(scriptor, xml_unescape_with_only_escapers)
{
    std::string xml = "&quot;&lt;&lt;&gt;&apos;&amp;&gt;&apos;&amp;";
    const std::string exp = "\"<<>'&>'&";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(scriptor, xml_unescape_with_escapers_and_random_ampersands)
{
    std::string xml = "&&quot;alb&lt;er&t&apos;eins&gt;&tein&amp;&";
    const std::string exp = "&\"alb<er&t'eins>&tein&&";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

TEST(scriptor, xml_unescape_with_half_escapers)
{
    std::string xml = "quot;alb&lt;ert&aos;eins&gt;tein&amp";
    const std::string exp = "quot;alb<ert&aos;eins>tein&amp";
    scriptor::xml_unescape(xml);
    ASSERT_EQ(exp, xml);
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
