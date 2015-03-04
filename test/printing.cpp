#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE printing
#include <boost/test/unit_test.hpp>

#include "../src/printing.hpp"
#include "../src/error/parse_error.hpp"
#include "../src/error/compile_exception.hpp"


#include <string>
#include <sstream>

using std::size_t;
using std::string;
using std::ostringstream;

using namespace parse_error;


BOOST_AUTO_TEST_CASE(default_error_stringifier_test)
{
    compile_exception exc{error_kind::PARSE, id("invalid_character"), code_location{{5, 2}, 1}, 5};
    
    auto file_id_to_name = [&](size_t file_id) -> string
    {
        BOOST_CHECK(file_id == 1);
        return "testmodule.al";
    };

    ostringstream oss;
    print_error(oss, exc, file_id_to_name);
    BOOST_CHECK_EQUAL(oss.str(), "testmodule.al:6:3: unexpected character: 5\n");
}

