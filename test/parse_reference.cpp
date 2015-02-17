#define BOOST_TEST_MODULE parse_reference
#include <boost/test/included/unit_test.hpp>

#include "../src/parse_reference.hpp"
#include "../src/parse_reference.hpp"

#include "symbol_building.hpp"
#include "state_utils.hpp"

using boost::optional;

BOOST_AUTO_TEST_CASE(letters)
{
    state s = make_state("abc");

    optional<symbol> got = parse_reference(s);
    BOOST_CHECK(got);
    symbol expected = sref("abc");

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == "");
}

BOOST_AUTO_TEST_CASE(digit_at_end)
{
    state s = make_state("abc9");

    optional<symbol> got = parse_reference(s);
    BOOST_CHECK(got);
    symbol expected = sref("abc9");

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == "");
}

BOOST_AUTO_TEST_CASE(operator_terminated)
{
    state s = make_state("ab[cd");

    optional<symbol> got = parse_reference(s);
    BOOST_CHECK(got);
    symbol expected = sref("ab");

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == "[cd");
}

BOOST_AUTO_TEST_CASE(digit_at_begin)
{
    state s = make_state("9abc");

    optional<symbol> got = parse_reference(s);
    BOOST_CHECK(!got);

    BOOST_CHECK(remaining(s) == "9abc");
}

BOOST_AUTO_TEST_CASE(brace_at_begin)
{
    state s = make_state("{abc");

    optional<symbol> got = parse_reference(s);
    BOOST_CHECK(!got);

    BOOST_CHECK(remaining(s) == "{abc");
}

BOOST_AUTO_TEST_CASE(operators)
{
    state s = make_state("$*&%<fw");

    optional<symbol> got = parse_reference(s);
    BOOST_CHECK(got);
    symbol expected = sref("$*&%<");

    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == "fw");
}

/*
    {
        string str = "$*&%<fw";
        state state(str.begin(), str.end());

        symbol result = *parse_reference(state);
        assert(boost::get<symbol::reference>(result.content).identifier == "$*&%<");
        assert(remaining(state) == "fw");
    }
}
*/
