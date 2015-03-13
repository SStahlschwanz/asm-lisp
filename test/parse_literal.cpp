#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE parse_literal
#include <boost/test/unit_test.hpp>

#include "../src/parse_literal.hpp"
#include "../src/parse_literal.hpp"

#include "../src/error/compile_exception.hpp"

#include "state_utils.hpp"

using std::string;

BOOST_AUTO_TEST_CASE(standard)
{
    state s = make_state("\"abc\"");

    lit_node* got = parse_literal(s);
    BOOST_CHECK(got);
    string expected = "abc";
    
    BOOST_CHECK(rangeify(expected) == rangeify(*got));
    BOOST_CHECK(remaining(s) == "");
}

BOOST_AUTO_TEST_CASE(digits)
{
    state s = make_state("912fas31");

    lit_node* got = parse_literal(s);
    BOOST_CHECK(got);
    string expected = "912";
    
    BOOST_CHECK(rangeify(*got) == rangeify(expected));
    BOOST_CHECK(remaining(s) == "fas31");
}

BOOST_AUTO_TEST_CASE(unterminated_quote)
{
    state s = make_state("\"abc");
    
    BOOST_CHECK_THROW(parse_literal(s), compile_exception);
}

BOOST_AUTO_TEST_CASE(quote_terminated_next_line)
{
    state s = make_state("\"abc\nasdf\"");
    
    BOOST_CHECK_THROW(parse_literal(s), compile_exception);
}

BOOST_AUTO_TEST_CASE(no_literal)
{
    state s = make_state("abc");

    lit_node* got = parse_literal(s);
    BOOST_CHECK(!got);
    BOOST_CHECK(remaining(s) == "abc");
}

