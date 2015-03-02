#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE parse_state
#include <boost/test/unit_test.hpp>

#include "../src/parse_state.hpp"
#include "../src/parse_state.hpp"

#include "state_utils.hpp"

BOOST_AUTO_TEST_CASE(state_test)
{
    state s = make_state("as\ndf");
    
    BOOST_CHECK(s.file() == default_file_id);

    BOOST_CHECK(!s.empty());
    BOOST_CHECK_EQUAL(s.position().line, 0);
    BOOST_CHECK_EQUAL(s.position().line_pos, 0);
    BOOST_CHECK_EQUAL(s.front(), 'a');
    
    s.pop_front();
    BOOST_CHECK(!s.empty());
    BOOST_CHECK_EQUAL(s.position().line, 0);
    BOOST_CHECK_EQUAL(s.position().line_pos, 1);
    BOOST_CHECK_EQUAL(s.front(), 's');

    s.pop_front();
    BOOST_CHECK(!s.empty());
    BOOST_CHECK_EQUAL(s.position().line, 0);
    BOOST_CHECK_EQUAL(s.position().line_pos, 2);
    BOOST_CHECK_EQUAL(s.front(), '\n');
    
    s.pop_front();
    BOOST_CHECK(!s.empty());
    BOOST_CHECK_EQUAL(s.position().line, 1);
    BOOST_CHECK_EQUAL(s.position().line_pos, 0);
    BOOST_CHECK_EQUAL(s.front(), 'd');

    s.pop_front();
    BOOST_CHECK(!s.empty());
    BOOST_CHECK_EQUAL(s.position().line, 1);
    BOOST_CHECK_EQUAL(s.position().line_pos, 1);
    BOOST_CHECK_EQUAL(s.front(), 'f');

    s.pop_front();
    BOOST_CHECK(s.empty());
}
