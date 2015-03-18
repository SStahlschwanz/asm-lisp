#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_type
#include <boost/test/unit_test.hpp>

#include "../src/compile_type.hpp"
#include "../src/error/compile_exception.hpp"
#include "../src/core_unique_ids.hpp"

#include "graph_building.hpp"
#include "state_utils.hpp"
#include "context.hpp"
#include "function_building.hpp"

#include <llvm/IR/DerivedTypes.h>

using std::vector;

using llvm::Type;
using llvm::IntegerType;
using llvm::PointerType;
using llvm::FunctionType;


id_node& int_id = id(unique_ids::INT);
id_node& ptr = id(unique_ids::PTR);

BOOST_AUTO_TEST_CASE(int_test)
{
    list_node& type1 = list{int_id, lit("65")};
    ref_node& type1_alias = ref("assd", &type1);
    type_info result = compile_type(type1_alias, context().llvm());
    BOOST_CHECK(&result.llvm_type == IntegerType::get(context().llvm(), 65));
    
    list_node& type2 = list{int_id, lit{"0"}};
    BOOST_CHECK_THROW(compile_type(type2, context().llvm()), compile_exception);
    
    list_node& type3 = list{int_id, lit{"-5"}};
    BOOST_CHECK_THROW(compile_type(type3, context().llvm()), compile_exception);
    
    list_node& type4 = list{int_id, lit{"53alksjdf"}};
    BOOST_CHECK_THROW(compile_type(type4, context().llvm()), compile_exception);
    
    list_node& type5 = list{int_id, ref{"53alksjdf"}};
    BOOST_CHECK_THROW(compile_type(type5, context().llvm()), compile_exception);
    
    list_node& type6 = list{int_id, lit{"53"}, ref("")};
    BOOST_CHECK_THROW(compile_type(type6, context().llvm()), compile_exception);
}

BOOST_AUTO_TEST_CASE(pointer_test)
{
    type_info result = compile_type(ptr, context().llvm());
    BOOST_CHECK(&result.llvm_type == PointerType::getUnqual(IntegerType::get(context().llvm(), 8)));
}

BOOST_AUTO_TEST_CASE(function_signature_test)
{
    list_node& int1 = list{int_id, lit{"1"}};
    list_node& int2 = list{int_id, lit{"2"}};
    list_node& signature1 = list{function_signature, list{int1}, int1};
    type_info info1 = compile_type(signature1, context().llvm());

    Type* llvm_int1 = IntegerType::get(context().llvm(), 1);
    vector<Type*> llvm_args1 = {llvm_int1};
    FunctionType* expected_llvm_type1 = FunctionType::get(llvm_int1, llvm_args1, false);
    BOOST_CHECK(&info1.llvm_type == expected_llvm_type1);


    list_node& signature2 = list{function_signature, list{int1, signature1}, int2};
    type_info info2 = compile_type(signature2, context().llvm());

    Type* llvm_int2 = IntegerType::get(context().llvm(), 2);
    vector<Type*> llvm_args2 = {llvm_int1, expected_llvm_type1};
    FunctionType* expected_llvm_type2 = FunctionType::get(llvm_int2, llvm_args2, false);
    BOOST_CHECK(&info2.llvm_type == expected_llvm_type2);

    type_info info3 = compile_type(sig_int64_2int64, context().llvm());

    Type* llvm_int64 = IntegerType::get(context().llvm(), 64);
    vector<Type*> llvm_args3 = {llvm_int64, llvm_int64};
    FunctionType* expected_llvm_type3 = FunctionType::get(llvm_int64, llvm_args3, false);
    BOOST_CHECK(&info3.llvm_type == expected_llvm_type3);
}

