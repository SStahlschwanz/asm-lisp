#define BOOST_TEST_MODULE parse_reference
#include <boost/test/included/unit_test.hpp>

#include "../src/parse_reference.hpp"
#include "../src/parse_reference.hpp"

#include "state_utils.hpp"

using boost::optional;

BOOST_AUTO_TEST_CASE(letters)
{
    state s = make_state("abc");

    optional<ref_symbol> got = parse_reference(s);
    BOOST_CHECK(got);
    ref_symbol expected{"abc"};

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == "");
}

BOOST_AUTO_TEST_CASE(digit_at_end)
{
    state s = make_state("abc9");

    optional<ref_symbol> got = parse_reference(s);
    BOOST_CHECK(got);
    ref_symbol expected{"abc9"};

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == "");
}

BOOST_AUTO_TEST_CASE(operator_terminated)
{
    state s = make_state("ab[cd");

    optional<ref_symbol> got = parse_reference(s);
    BOOST_CHECK(got);
    ref_symbol expected{"ab"};

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == "[cd");
}

BOOST_AUTO_TEST_CASE(digit_at_begin)
{
    state s = make_state("9abc");

    optional<ref_symbol> got = parse_reference(s);
    BOOST_CHECK(!got);

    BOOST_CHECK(remaining(s) == "9abc");
}

BOOST_AUTO_TEST_CASE(brace_at_begin)
{
    state s = make_state("{abc");

    optional<ref_symbol> got = parse_reference(s);
    BOOST_CHECK(!got);

    BOOST_CHECK(remaining(s) == "{abc");
}

BOOST_AUTO_TEST_CASE(operators)
{
    state s = make_state("$*&%<fw");

    optional<ref_symbol> got = parse_reference(s);
    BOOST_CHECK(got);
    ref_symbol expected{"$*&%<"};

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == "fw");
}
