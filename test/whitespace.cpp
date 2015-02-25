#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Hello
#include <boost/test/unit_test.hpp>

#include "../src/whitespace.hpp"
#include "../src/whitespace.hpp"

#include "state_utils.hpp"

#include <cassert>
#include <string>


BOOST_AUTO_TEST_CASE(standard)
{
    state s = make_state(" \n\t");
    
    BOOST_CHECK(whitespace(s));
    BOOST_CHECK(remaining(s) == "");
}

BOOST_AUTO_TEST_CASE(letter_at_begin)
{
    state s = make_state("f \n\t");
    
    BOOST_CHECK(!whitespace(s));
    BOOST_CHECK(remaining(s) == "f \n\t");
}

BOOST_AUTO_TEST_CASE(letter_in_middle)
{
    state s = make_state("   f ");
    
    BOOST_CHECK(whitespace(s));
    BOOST_CHECK(remaining(s) == "f ");
}

