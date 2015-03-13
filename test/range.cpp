#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE range
#include <boost/test/unit_test.hpp>

#include "../src/range.hpp"

#include <vector>
#include <string>

using std::vector;
using std::string;

BOOST_AUTO_TEST_CASE(comparison_test)
{
    string str = "3452fasf";
    vector<char> vec{str.begin(), str.end()};
    string str2 = "asdfasdfasfasf";
    BOOST_CHECK(rangeify(str) == rangeify(vec));
    BOOST_CHECK(rangeify(str2) != rangeify(vec));
}
