#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE parse
#include <boost/test/unit_test.hpp>

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
    any_symbol expected = ref("abcd"_id);
    
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
    any_symbol expected = list{list{ref("abc"_id)}, list{lit("fff"), ref("def"_id)}, list{}};

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " efg");
}

BOOST_AUTO_TEST_CASE(square_list_test)
{
    state s = make_state("[ef ak, [a ], [ ] ,] ff");

    optional<any_symbol> got = parse_node(s);
    BOOST_CHECK(got);
    any_symbol expected = list{list{ref("ef"_id), ref("ak"_id)}, 
            list{list{list{ref("a"_id)}}}, list{list{list{}}}, list{}};

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " ff");
}

BOOST_AUTO_TEST_CASE(round_list_test)
{
    state s = make_state("( ff, aa (a b) , (),) ff");

    optional<any_symbol> got = parse_node(s);
    BOOST_CHECK(got);
    any_symbol expected = list{list{ref("ff"_id)}, list{ref("aa"_id), list{ref("a"_id), ref("b"_id)}},
            list{list{}}, list{}};

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == " ff");
}

const state mod1 = make_state(
R"(abcde {} fer;
fj;
+-* 123;)");

BOOST_AUTO_TEST_CASE(parse_file_test1)
{
    state s{mod1};
    optional<list_symbol> got = parse_file(s);
    BOOST_CHECK(got);
    any_symbol expected = list{
        list{ref{"abcde"_id}, list{}, ref{"fer"_id}},
        list{ref{"fj"_id}},
        list{ref{"+-*"_id}, lit{"123"}}};
    
    BOOST_CHECK(*got == expected);
}

const state mod2 = make_state(
R"(import (a b c) from a;)"); 

BOOST_AUTO_TEST_CASE(parse_file_test2)
{
    state s{mod2};
    optional<list_symbol> got = parse_file(s);
    BOOST_CHECK(got);
    any_symbol expected = list{
        list{ref{"import"_id}, list{ref{"a"_id}, ref{"b"_id}, ref{"c"_id}}, ref{"from"_id}, ref{"a"_id}}};
    
    BOOST_CHECK(*got == expected);
}

