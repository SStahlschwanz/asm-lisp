#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Hello
#include <boost/test/unit_test.hpp>

#include "../src/parse_literal.hpp"
#include "../src/parse_literal.hpp"

#include "state_utils.hpp"

using boost::optional;

BOOST_AUTO_TEST_CASE(standard)
{
    state s = make_state("\"abc\"");

    optional<lit_symbol> got = parse_literal(s);
    BOOST_CHECK(got);
    lit_symbol expected = lit_symbol("abc");
    
    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == "");
}

BOOST_AUTO_TEST_CASE(digits)
{
    state s = make_state("912fas31");

    optional<lit_symbol> got = parse_literal(s);
    BOOST_CHECK(got);
    lit_symbol expected = lit_symbol("912");
    
    BOOST_CHECK(*got == expected);
    BOOST_CHECK(remaining(s) == "fas31");
}

BOOST_AUTO_TEST_CASE(unterminated_quote)
{
    state s = make_state("\"abc");
    
    BOOST_CHECK_THROW(parse_literal(s), parse_exception);
}

BOOST_AUTO_TEST_CASE(quote_terminated_next_line)
{
    state s = make_state("\"abc\nasdf\"");
    
    BOOST_CHECK_THROW(parse_literal(s), parse_exception);
}

BOOST_AUTO_TEST_CASE(no_literal)
{
    state s = make_state("abc");

    optional<lit_symbol> got = parse_literal(s);
    BOOST_CHECK(!got);
    BOOST_CHECK(remaining(s) == "abc");
}

