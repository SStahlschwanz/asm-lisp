#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_macro
#include <boost/test/unit_test.hpp>

#include "../src/compile_macro.hpp"
#include "../src/core_unique_ids.hpp"
#include "../src/error/compile_exception.hpp"

#include "state_utils.hpp"

#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <iterator>

using std::pair;
using std::unique_ptr;
using std::unordered_map;
using std::advance;

using llvm::Function;
using llvm::Value;
using llvm::IntegerType;
using llvm::Type;

using boost::get;

using namespace symbol_shortcuts;

typedef lit_symbol lit;
typedef ref_symbol ref;
typedef list_symbol list;


#include <iostream>
/*
BOOST_AUTO_TEST_CASE(compile_signature_test)
{
    const type_symbol int64_type{IntegerType::get(context.llvm(), 64)};
    const ref a{"a"_id};
    const ref b{"b"_id};
    const ref c{"c"_id};
    

    const any_symbol params1 = list
    {
        list{a, int64_type},
        list{b, int64_type},
        list{c, int64_type},
    };
    const any_symbol return_type1 = int64_type;
    
    Function* function1;
    unordered_map<identifier_id_t, named_value_info> parameter_table1;
    tie(function1, parameter_table1) = compile_signature(params1, return_type1, context);
    
    BOOST_CHECK(function1 != nullptr);
    BOOST_CHECK_EQUAL(parameter_table1.size(), 3);
    BOOST_CHECK(parameter_table1.count(a.identifier()));
    BOOST_CHECK(parameter_table1.count(b.identifier()));
    BOOST_CHECK(parameter_table1.count(c.identifier()));
    BOOST_CHECK(parameter_table1.at(b.identifier()).llvm_value == (++function1->arg_begin()));
    
    const any_symbol params2 = list
    {
        list{a, int64_type},
        list{b, int64_type},
        list{a, int64_type}, // duplicate parameter name
    };
    const any_symbol return_type2 = int64_type;
    
    BOOST_CHECK_THROW(compile_signature(params2, return_type2, context), compile_exception);
}

BOOST_AUTO_TEST_CASE(compile_instruction_test)
{
    Type* llvm_int64 = IntegerType::get(context.llvm(), 64);
    const type_symbol int64_type{llvm_int64};
    
    const id_symbol add_constructor{unique_ids::ADD};
    const list_symbol instruction1{add_constructor, int64_type}; 
    const instruction_statement got1 = compile_instruction(instruction1);
    BOOST_CHECK(get<instruction_statement::add>(got1.instruction).type == int64_type);
    
    const id_symbol div_constructor{unique_ids::DIV};
    const list_symbol instruction2{div_constructor, int64_type}; 
    const instruction_statement got2 = compile_instruction(instruction2);
    BOOST_CHECK(get<instruction_statement::div>(got2.instruction).type == int64_type);

    const id_symbol cmp_constructor{unique_ids::CMP};
    const ref_symbol cmp_ref{"asdfasdf"_id, &cmp_constructor};
    const id_symbol lt{unique_ids::LT};
    const list_symbol instruction3{cmp_ref, lt, int64_type};
    const instruction_statement got3 = compile_instruction(instruction3);
    BOOST_CHECK(get<instruction_statement::cmp>(got3.instruction).cmp_kind == unique_ids::LT);
    BOOST_CHECK(get<instruction_statement::cmp>(got3.instruction).type == int64_type);
}
*/
BOOST_AUTO_TEST_CASE(compile_macro_test)
{
    const type_symbol int64_type{IntegerType::get(context.llvm(), 64)};
    const ref a{"a"_id};
    const ref b{"b"_id};
    const ref c{"c"_id};
    
    const list_symbol params =
    {
        list{a, int64_type},
        list{b, int64_type},
        list{c, int64_type}
    };
    const type_symbol return_type = int64_type;
    
    const list_symbol add_int64 = {id_symbol{unique_ids::ADD}, int64_type};
    const list_symbol sub_int64 = {id_symbol{unique_ids::SUB}, int64_type};
    const id_symbol let{unique_ids::LET};
    const ref block1{"block1"_id};
    const ref d{"d"_id};
    const ref e{"e"_id};
    const ref k{"k"_id};
    const list_symbol return_int64 = {id_symbol{unique_ids::RETURN}, int64_type};
    const list_symbol alloc_int64 = {id_symbol{unique_ids::ALLOC}, int64_type};
    const list_symbol body =
    {
        list{block1, list
        {
            list{let, d, add_int64, a, b},
            list{let, e, add_int64, d, c},
            list{let, k, alloc_int64},
            list{return_int64, e}
        }}
    };
    const list_symbol macro =
    {
        params,
        return_type,
        body
    };

    Function* function = compile_macro(macro.begin(), macro.end(), context);
    auto fptr = (uint64_t (*)(uint64_t, uint64_t, uint64_t)) context.llvm_execution_engine().getPointerToFunction(function);

    BOOST_CHECK(fptr);
    std::cout << fptr(2, 5, 9) << std::endl;
    function->dump();
    /*
    void* compiled_func = context.llvm_execution_engine().getPointerToFunction(function);
    BOOST_CHECK(compiled_func);
    */
}

