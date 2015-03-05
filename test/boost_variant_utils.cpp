#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE boost_variant_utils
#include <boost/test/unit_test.hpp>

#include "../src/boost_variant_utils.hpp"

#include <string>

using boost::variant;
using std::string;


BOOST_AUTO_TEST_CASE(test)
{
    variant<int, char, float> var;
    
    var = (int)5;
    
    visit(var, 
    [&](int i)
    {
        BOOST_CHECK_EQUAL(i, 5);
    },
    [&](char)
    {
        BOOST_ERROR("");
    },
    [&](float)
    {
        BOOST_ERROR("");
    });

    var = (char)'a';

    string result = visit<string>(var,
    [](int) -> string
    {
        BOOST_ERROR("");
        return "";
    },
    [](auto c) -> string
    {
        return string{c};
    },
    [](float) -> string
    {
        BOOST_ERROR("");
        return "";
    });
    BOOST_CHECK(result == "a");
}
