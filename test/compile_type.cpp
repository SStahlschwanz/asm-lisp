#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_type
#include <boost/test/unit_test.hpp>

#include "../src/compile_type.hpp"
#include "../src/error/compile_exception.hpp"
#include "../src/core_unique_ids.hpp"

#include "state_utils.hpp"

#include <llvm/IR/DerivedTypes.h>

using llvm::IntegerType;

using namespace symbol_shortcuts;


const id_symbol int_id{unique_ids::INT};

BOOST_AUTO_TEST_CASE(int_test)
{
    compilation_context context;
    
    const list type1{int_id, lit{"65"}};
    const ref type1_alias{"assd"_id, &type1};
    type_info result = read_type(type1_alias, context.llvm());
    BOOST_CHECK(result.llvm_type == IntegerType::get(context.llvm(), 65));
    
    const list type2{int_id, lit_symbol{"0"}};
    BOOST_CHECK_THROW(read_type(type2, context.llvm()), compile_exception);
    
    const list type3{int_id, lit_symbol{"-5"}};
    BOOST_CHECK_THROW(read_type(type3, context.llvm()), compile_exception);
    
    const list type4{int_id, lit_symbol{"53alksjdf"}};
    BOOST_CHECK_THROW(read_type(type4, context.llvm()), compile_exception);
    
    const list type5{int_id, ref_symbol{"53alksjdf"_id}};
    BOOST_CHECK_THROW(read_type(type5, context.llvm()), compile_exception);
    
    const list type6{int_id, lit_symbol{"53"}, ref_symbol{""_id}};
    BOOST_CHECK_THROW(read_type(type6, context.llvm()), compile_exception);
}

