#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE parse_reference
#include <boost/test/unit_test.hpp>

#include "../src/parse_reference.hpp"
#include "../src/parse_reference.hpp"

#include "state_utils.hpp"

using std::string;

BOOST_AUTO_TEST_CASE(letters)
{
    state s = make_state("abc");

    ref_node* got = parse_reference(s);
    BOOST_CHECK(got);
    string expected = "abc";

    BOOST_CHECK(got->identifier() == rangeify(expected));
    BOOST_CHECK(remaining(s) == "");
}
BOOST_AUTO_TEST_CASE(underscore)
{
    state s = make_state("_a_bc");

    ref_node* got = parse_reference(s);
    BOOST_CHECK(got);
    string expected = "_a_bc";

    BOOST_CHECK(got->identifier() == rangeify(expected));
    BOOST_CHECK(remaining(s) == "");
}

BOOST_AUTO_TEST_CASE(digit_at_end)
{
    state s = make_state("abc9");

    ref_node* got = parse_reference(s);
    BOOST_CHECK(got);
    string expected = "abc9";

    BOOST_CHECK(got->identifier() == rangeify(expected));
    BOOST_CHECK(remaining(s) == "");
}

BOOST_AUTO_TEST_CASE(operator_terminated)
{
    state s = make_state("ab[cd");

    ref_node* got = parse_reference(s);
    BOOST_CHECK(got);

    string expected = "ab";

    BOOST_CHECK(got->identifier() == rangeify(expected));
    BOOST_CHECK(remaining(s) == "[cd");
}

BOOST_AUTO_TEST_CASE(digit_at_begin)
{
    state s = make_state("9abc");

    ref_node* got = parse_reference(s);
    BOOST_CHECK(!got);

    BOOST_CHECK(remaining(s) == "9abc");
}

BOOST_AUTO_TEST_CASE(brace_at_begin)
{
    state s = make_state("{abc");

    ref_node* got = parse_reference(s);
    BOOST_CHECK(!got);

    BOOST_CHECK(remaining(s) == "{abc");
}

BOOST_AUTO_TEST_CASE(operators)
{
    state s = make_state("$*&%<fw");

    ref_node* got = parse_reference(s);
    BOOST_CHECK(got);
    string expected = "$*&%<";

    BOOST_CHECK(got->identifier() == rangeify(expected));
    BOOST_CHECK(remaining(s) == "fw");
}

