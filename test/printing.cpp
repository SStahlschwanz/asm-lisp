#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE printing
#include <boost/test/unit_test.hpp>

#include "../src/printing.hpp"
#include "../src/error/parse_error.hpp"
#include "../src/error/compile_function_error.hpp"
#include "../src/error/compile_exception.hpp"


#include <string>
#include <sstream>

using std::size_t;
using std::string;
using std::ostringstream;


string file_id_to_name(size_t file_id)
{
    BOOST_CHECK(file_id == 1);
    return "testmodule.al";
};

BOOST_AUTO_TEST_CASE(error_lookup_printing_test)
{
    using namespace parse_error;
    compile_exception exc{error_kind::PARSE, id("invalid_character"), code_location{{5, 2}, 1}, 5};
    

    ostringstream oss;
    print(oss, exc, file_id_to_name);
    BOOST_CHECK_EQUAL(oss.str(), "testmodule.al:6:3: unexpected character: 5\n");
}

BOOST_AUTO_TEST_CASE(missing_error_message_test)
{
    using namespace compile_function_error;
    compile_exception exc{error_kind::COMPILE_FUNCTION, id("empty_statement"), code_location{{5, 2}, 1}};

    ostringstream oss;
    print(oss, exc, file_id_to_name);
    BOOST_CHECK_EQUAL(oss.str(), "testmodule.al:6:3: <empty_statement> (missing error message)\n");
}


