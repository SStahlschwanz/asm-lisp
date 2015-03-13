#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE parse
#include <boost/test/unit_test.hpp>

#include "../src/parse.hpp"

#include "state_utils.hpp"


BOOST_AUTO_TEST_CASE(identifier_test)
{
    state s = make_state("abcd efg");

    node* got = parse_node(s);
    BOOST_CHECK(got);

    node& expected = ref("abcd");
    
    BOOST_CHECK(structurally_equal(*got, expected));
    BOOST_CHECK(remaining(s) == " efg");
}

BOOST_AUTO_TEST_CASE(literal_test)
{
    state s = make_state("\"abcd\" efg");

    node* got = parse_node(s);
    BOOST_CHECK(got);
    
    node& expected = lit("abcd");

    BOOST_CHECK(structurally_equal(*got, expected));
    BOOST_CHECK(remaining(s) == " efg");
}

BOOST_AUTO_TEST_CASE(curly_list_test)
{
    state s = make_state("{ abc; \"fff\" def; ; } efg");

    node* got = parse_node(s);
    BOOST_CHECK(got);
    
    node& expected = list{list{ref("abc")}, list{lit("fff"), ref("def")}, list{}};

    BOOST_CHECK(structurally_equal(*got, expected));
    BOOST_CHECK(remaining(s) == " efg");
}

BOOST_AUTO_TEST_CASE(round_list_test)
{
    state s = make_state("( ff aa (a b)  ()) ff");

    node* got = parse_node(s);
    BOOST_CHECK(got);
    
    node& expected = list{ref("ff"), ref("aa"), list{ref("a"), ref("b")}, list{}};

    BOOST_CHECK(structurally_equal(*got, expected));
    BOOST_CHECK(remaining(s) == " ff");
}

const state mod1 = make_state(
R"(abcde {} fer;
fj;
+-* 123;)");

BOOST_AUTO_TEST_CASE(parse_file_test1)
{
    state s{mod1};
    node* got = parse_file(s);
    BOOST_CHECK(got);
    
    node& expected = list
    {
        list{ref{"abcde"}, list{}, ref{"fer"}},
        list{ref{"fj"}},
        list{ref{"+-*"}, lit{"123"}}
    };
    
    BOOST_CHECK(structurally_equal(*got, expected));
}

const state mod2 = make_state(
R"(import (a b c) from a;)"); 

BOOST_AUTO_TEST_CASE(parse_file_test2)
{
    state s{mod2};
    node* got = parse_file(s);
    BOOST_CHECK(got);
    
    node& expected = list
    {
        list{ref{"import"}, list{ref{"a"}, ref{"b"}, ref{"c"}}, ref{"from"}, ref{"a"}}
    };
    
    BOOST_CHECK(structurally_equal(*got, expected));
}

