#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE error_to_string
#include <boost/test/unit_test.hpp>

#include "../src/error_to_string.hpp"

#include <unordered_map>
#include <string>

using std::unordered_map;
using std::size_t;
using std::string;


BOOST_AUTO_TEST_CASE(default_error_stringifier_test)
{
    unordered_map<size_t, string> file_id_name_mapping;
    file_id_name_mapping[1] = "testmodule.al";
    compile_exception exc{"parse_invalid_character", code_location{file_position{5, 2}, 1}, "c"};
    string error = default_error_to_string(exc, file_id_name_mapping);
    BOOST_CHECK_EQUAL(error, "testmodule.al:6:3: unexpected character 'c'");
}

