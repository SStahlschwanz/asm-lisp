#define BOOST_TEST_MODULE parse
#include <boost/test/included/unit_test.hpp>

#include "../src/parse.hpp"

#include "state_utils.hpp"

using boost::optional;

typedef lit_symbol lit;
typedef ref_symbol ref;
typedef list_symbol list;

BOOST_AUTO_TEST_CASE(identifier_test)
{
    state s = make_state("abcd efg");

    optional<any_symbol> got = parse_node(s);
    BOOST_CHECK(got);
    any_symbol expected = ref("abcd");
    
    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " efg");
}

BOOST_AUTO_TEST_CASE(literal_test)
{
    state s = make_state("\"abcd\" efg");

    optional<any_symbol> got = parse_node(s);
    BOOST_CHECK(got);
    any_symbol expected = lit("abcd");

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " efg");
}

BOOST_AUTO_TEST_CASE(curly_list_test)
{
    state s = make_state("{ abc; \"fff\" def; ; } efg");

    optional<any_symbol> got = parse_node(s);
    BOOST_CHECK(got);
    any_symbol expected = list{list{ref("abc")}, list{lit("fff"), ref("def")}, list{}};

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " efg");
}

BOOST_AUTO_TEST_CASE(square_list_test)
{
    state s = make_state("[ef ak, [a ], [ ] ,] ff");

    optional<any_symbol> got = parse_node(s);
    BOOST_CHECK(got);
    any_symbol expected = list{list{ref("ef"), ref("ak")}, 
            list{list{list{ref("a")}}}, list{list{list{}}}, list{}};

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " ff");
}

BOOST_AUTO_TEST_CASE(round_list_test)
{
    state s = make_state("( ff, aa (a b) , (),) ff");

    optional<any_symbol> got = parse_node(s);
    BOOST_CHECK(got);
    any_symbol expected = list{list{ref("ff")}, list{ref("aa"), list{ref("a"), ref("b")}},
            list{list{}}, list{}};

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " ff");
}

const state mod1 = make_state(
R"(abcde {} fer;
fj;
+-* 123;)");

BOOST_AUTO_TEST_CASE(parse_file_test)
{
    state s{mod1};
    optional<list_symbol> got = parse_file(s);
    BOOST_CHECK(got);
    any_symbol expected = list{
        list{ref{"abcde"}, list{}, ref{"fer"}},
        list{ref{"fj"}},
        list{ref{"+-*"}, lit{"123"}}};
    
    BOOST_CHECK(*got == expected);
}

