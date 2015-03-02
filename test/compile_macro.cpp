#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_macro
#include <boost/test/unit_test.hpp>

#include "../src/compile_macro.hpp"
#include "../src/error/compile_macro_exception.hpp"
#include "../src/core_unique_ids.hpp"

#include "state_utils.hpp"

#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>

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
    
    unique_ptr<Function> function1;
    unordered_map<identifier_id_t, value_info> parameter_table1;
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
    
    BOOST_CHECK_THROW(compile_signature(params2, return_type2, context), compile_macro_exception::compile_macro_exception);
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

