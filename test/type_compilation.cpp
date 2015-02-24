#define BOOST_TEST_MODULE type_compilation
#include <boost/test/included/unit_test.hpp>

#include "../src/type_compilation.hpp"
#include "../src/error/type_compilation_exception.hpp"

#include <llvm/IR/DerivedTypes.h>

using llvm::IntegerType;

#include <iostream>
using namespace std;

BOOST_AUTO_TEST_CASE(int_test)
{
    compilation_context context;

    const list_symbol param1{lit_symbol{"65"}};
    type_symbol result = compile_int(param1.begin(), param1.end(), context);
    type_symbol expected{IntegerType::get(context.llvm(), 65)};
    BOOST_CHECK(result == expected);
    
    const list_symbol param2{lit_symbol{"0"}};
    BOOST_CHECK_THROW(compile_int(param2.begin(), param2.end(), context), type_compilation_exception);
    
    const list_symbol param3{lit_symbol{"-5"}};
    BOOST_CHECK_THROW(compile_int(param3.begin(), param3.end(), context), type_compilation_exception);
    
    const list_symbol param4{lit_symbol{"53alksjdf"}};
    BOOST_CHECK_THROW(compile_int(param4.begin(), param4.end(), context), type_compilation_exception);
    
    const list_symbol param5{ref_symbol{"53alksjdf"}};
    BOOST_CHECK_THROW(compile_int(param5.begin(), param5.end(), context), type_compilation_exception);
    
    const list_symbol param6{lit_symbol{"53"}, ref_symbol{""}};
    BOOST_CHECK_THROW(compile_int(param6.begin(), param6.end(), context), type_compilation_exception);
}

