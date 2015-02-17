#define BOOST_TEST_MODULE parse
#include <boost/test/included/unit_test.hpp>

#include "../src/parse.hpp"

#include "symbol_building.hpp"
#include "state_utils.hpp"

using boost::optional;

BOOST_AUTO_TEST_CASE(identifier_test)
{
    state s = make_state("abcd efg");

    optional<symbol> got = parse_node(s);
    BOOST_CHECK(got);
    symbol expected = sref("abcd");
    
    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " efg");
}

BOOST_AUTO_TEST_CASE(literal_test)
{
    state s = make_state("\"abcd\" efg");

    optional<symbol> got = parse_node(s);
    BOOST_CHECK(got);
    symbol expected = lit("abcd");

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " efg");
}

BOOST_AUTO_TEST_CASE(curly_list_test)
{
    state s = make_state("{ abc; \"fff\" def; ; } efg");

    optional<symbol> got = parse_node(s);
    BOOST_CHECK(got);
    symbol expected = list(list(sref("abc")), list(lit("fff"), sref("def")), list());

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " efg");
}

BOOST_AUTO_TEST_CASE(square_list_test)
{
    state s = make_state("[ef ak, [a ], [ ] ,] ff");

    optional<symbol> got = parse_node(s);
    BOOST_CHECK(got);
    symbol expected = list(list(sref("ef"), sref("ak")), 
            list(list(list(sref("a")))), list(list(list())), list());

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " ff");
}

BOOST_AUTO_TEST_CASE(round_list_test)
{
    state s = make_state("( ff, aa (a b) , (),) ff");

    optional<symbol> got = parse_node(s);
    BOOST_CHECK(got);
    symbol expected = list(list(sref("ff")), list(sref("aa"), list(sref("a"), sref("b"))),
            list(list()), list());

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " ff");
}

