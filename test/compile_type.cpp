#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_type
#include <boost/test/unit_test.hpp>

#include "../src/compile_type.hpp"
#include "../src/error/compile_exception.hpp"
#include "../src/core_unique_ids.hpp"

#include "state_utils.hpp"

#include <llvm/IR/DerivedTypes.h>

using std::vector;

using llvm::Type;
using llvm::IntegerType;
using llvm::PointerType;
using llvm::FunctionType;

using namespace symbol_shortcuts;


const id_symbol int_id{unique_ids::INT};
const id_symbol ptr{unique_ids::PTR};
const id_symbol function_signature{unique_ids::FUNCTION_SIGNATURE};

BOOST_AUTO_TEST_CASE(int_test)
{
    const list type1{int_id, lit{"65"}};
    const ref type1_alias{"assd"_id, &type1};
    type_info result = compile_type(type1_alias, context.llvm());
    BOOST_CHECK(result.llvm_type == IntegerType::get(context.llvm(), 65));
    
    const list type2{int_id, lit_symbol{"0"}};
    BOOST_CHECK_THROW(compile_type(type2, context.llvm()), compile_exception);
    
    const list type3{int_id, lit_symbol{"-5"}};
    BOOST_CHECK_THROW(compile_type(type3, context.llvm()), compile_exception);
    
    const list type4{int_id, lit_symbol{"53alksjdf"}};
    BOOST_CHECK_THROW(compile_type(type4, context.llvm()), compile_exception);
    
    const list type5{int_id, ref_symbol{"53alksjdf"_id}};
    BOOST_CHECK_THROW(compile_type(type5, context.llvm()), compile_exception);
    
    const list type6{int_id, lit_symbol{"53"}, ref_symbol{""_id}};
    BOOST_CHECK_THROW(compile_type(type6, context.llvm()), compile_exception);
}

BOOST_AUTO_TEST_CASE(pointer_test)
{
    const list type1{ptr};
    type_info result = compile_type(type1, context.llvm());
    BOOST_CHECK(result.llvm_type == PointerType::getUnqual(IntegerType::get(context.llvm(), 8)));
}

BOOST_AUTO_TEST_CASE(function_signature_test)
{
    const list int1{int_id, lit{"1"}};
    const list int2{int_id, lit{"2"}};
    const list signature1{function_signature, int1, list{int1}};
    type_info info1 = compile_type(signature1, context.llvm());

    Type* llvm_int1 = IntegerType::get(context.llvm(), 1);
    vector<Type*> llvm_args1 = {llvm_int1};
    FunctionType* expected_llvm_type1 = FunctionType::get(llvm_int1, llvm_args1, false);
    BOOST_CHECK(info1.llvm_type == expected_llvm_type1);


    const list signature2{function_signature, int2, list{int1, signature1}};
    type_info info2 = compile_type(signature2, context.llvm());

    Type* llvm_int2 = IntegerType::get(context.llvm(), 2);
    vector<Type*> llvm_args2 = {llvm_int1, expected_llvm_type1};
    FunctionType* expected_llvm_type2 = FunctionType::get(llvm_int2, llvm_args2, false);
    BOOST_CHECK(info2.llvm_type == expected_llvm_type2);
}
