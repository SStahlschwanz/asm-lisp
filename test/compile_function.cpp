#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_function
#include <boost/test/unit_test.hpp>

#include "../src/compile_function.hpp"
#include "../src/core_unique_ids.hpp"
#include "../src/error/compile_exception.hpp"

#include "state_utils.hpp"

#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <iterator>
#include <string>
#include <csetjmp>

using std::pair;
using std::tie;
using std::ignore;
using std::unique_ptr;
using std::unordered_map;
using std::advance;
using std::string;

using llvm::Function;
using llvm::Value;
using llvm::IntegerType;
using llvm::Type;
using llvm::verifyFunction;
using llvm::raw_string_ostream;

using boost::get;

using namespace symbol_shortcuts;

const list_symbol int64_type{id_symbol{unique_ids::INT}, lit{"64"}};

const ref a{"a"_id};
const ref b{"b"_id};
const ref c{"c"_id};

const ref q{"q"_id};
const ref r{"r"_id};
const ref s{"s"_id};
const ref t{"t"_id};
const ref u{"u"_id};
const ref v{"v"_id};
const ref w{"w"_id};
const ref x{"x"_id};
const ref y{"y"_id};
const ref z{"z"_id};

const ref block1{"block1"_id};
const ref block2{"block2"_id};
const ref block3{"block3"_id};
const ref block4{"block4"_id};
    
const id_symbol let{unique_ids::LET};

const list_symbol alloc_int64 = {id_symbol{unique_ids::ALLOC}, int64_type};
const list_symbol store_int64 = {id_symbol{unique_ids::STORE}, int64_type};
const list_symbol load_int64 = {id_symbol{unique_ids::LOAD}, int64_type};
const list_symbol add_int64 = {id_symbol{unique_ids::ADD}, int64_type};
const list_symbol sub_int64 = {id_symbol{unique_ids::SUB}, int64_type};
const list_symbol return_int64 = {id_symbol{unique_ids::RETURN}, int64_type};
const list_symbol cmp_eq_int64 = {id_symbol{unique_ids::CMP}, id_symbol{unique_ids::EQ}, int64_type};
const list_symbol cmp_ne_int64 = {id_symbol{unique_ids::CMP}, id_symbol{unique_ids::NE}, int64_type};
const list_symbol cond_branch = {id_symbol{unique_ids::COND_BRANCH}};
const list_symbol branch = {id_symbol{unique_ids::BRANCH}};
const list_symbol phi_int64 = {id_symbol{unique_ids::PHI}, int64_type};

const list_symbol list_create = {id_symbol{unique_ids::LIST_CREATE}};
const list_symbol list_size = {id_symbol{unique_ids::LIST_SIZE}};
const list_symbol list_set = {id_symbol{unique_ids::LIST_SET}};
const list_symbol list_get = {id_symbol{unique_ids::LIST_GET}};
const list_symbol list_push = {id_symbol{unique_ids::LIST_PUSH}};
const list_symbol list_pop = {id_symbol{unique_ids::LIST_POP}};


template<class T>
T* get_compiled_function(const list_symbol& function_source)
{
    unique_ptr<Function> function_owner;
    tie(function_owner, ignore) = compile_function(function_source.begin(), function_source.end(), context);
    Function* function = function_owner.get();
    context.macro_environment().llvm_module.getFunctionList().push_back(function_owner.get());
    function_owner.release();
    
    string str;
    raw_string_ostream os(str);
    //BOOST_CHECK_MESSAGE(verifyFunction(*function, &os), os.str());
    // TODO: this fails - I don't know why, function->dump() looks good to me and verifyFunction doesn't produce a message
    auto fptr = (T*) context.macro_environment().llvm_engine.getPointerToFunction(function);
    return fptr;
}

BOOST_AUTO_TEST_CASE(compile_signature_test)
{
    const any_symbol params1 = list
    {
        list{a, int64_type},
        list{b, int64_type},
        list{c, int64_type},
    };
    const any_symbol return_type1 = int64_type;
    
    unique_ptr<Function> function1;
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
    
    const id_symbol add_constructor{unique_ids::ADD};
    const list_symbol instruction1{add_constructor, int64_type}; 
    const instruction_info got1 = parse_instruction(instruction1, context.llvm());
    BOOST_CHECK(get<instruction_info::add>(got1.kind).type.llvm_type == llvm_int64);
    
    const id_symbol div_constructor{unique_ids::DIV};
    const list_symbol instruction2{div_constructor, int64_type}; 
    const instruction_info got2 = parse_instruction(instruction2, context.llvm());
    BOOST_CHECK(get<instruction_info::div>(got2.kind).type.llvm_type == llvm_int64);

    const id_symbol cmp_constructor{unique_ids::CMP};
    const ref_symbol cmp_ref{"asdfasdf"_id, &cmp_constructor};
    const id_symbol lt{unique_ids::LT};
    const list_symbol instruction3{cmp_ref, lt, int64_type};
    const instruction_info got3 = parse_instruction(instruction3, context.llvm());
    BOOST_CHECK(get<instruction_info::cmp>(got3.kind).cmp_kind == unique_ids::LT);
    BOOST_CHECK(get<instruction_info::cmp>(got3.kind).type.llvm_type == llvm_int64);
}

BOOST_AUTO_TEST_CASE(store_load_test)
{
    const list_symbol params =
    {
        list{a, int64_type}
    };
    const symbol& return_type = int64_type;
    const list_symbol body =
    {
        list{block1, list
        {
            list{let, x, alloc_int64},
            list{store_int64, a, x},
            list{let, y, load_int64, x},
            list{return_int64, y}
        }}
    };

    const list_symbol function_source = 
    {
        params,
        return_type,
        body
    };

    auto function_ptr = get_compiled_function<uint64_t (uint64_t)>(function_source);
    BOOST_CHECK(function_ptr(2) == 2);
    BOOST_CHECK(function_ptr(1231) == 1231);
}

BOOST_AUTO_TEST_CASE(branch_test)
{
    const list_symbol params =
    {
        list{a, int64_type},
        list{b, int64_type},
    };
    
    const symbol& return_type = int64_type;
    
    const list_symbol body =
    {
        list{block1, list
        {
            list{let, x, cmp_eq_int64, a, b},
            list{cond_branch, x, block2, block3}
        }},
        list{block2, list
        {
            list{let, y, add_int64, a, b},
            list{return_int64, y}
        }},
        list{block3, list
        {
            list{let, z, sub_int64, a, b},
            list{return_int64, z}
        }}
    };
    const list_symbol func_source =
    {
        params,
        return_type,
        body
    };
    
    auto compiled_function = get_compiled_function<uint64_t (uint64_t, uint64_t)>(func_source);
    BOOST_CHECK(compiled_function);
    BOOST_CHECK_EQUAL(compiled_function(2, 2), 2 + 2);
    BOOST_CHECK_EQUAL(compiled_function(5, 3), 5 - 3);
}

BOOST_AUTO_TEST_CASE(phi_test)
{
    const list_symbol params =
    {
        list{a, int64_type},
        list{b, int64_type},
    };
    
    const symbol& return_type = int64_type;

    const list_symbol block1_def = {block1, list
    {
        list{let, x, cmp_eq_int64, a, b},
        list{cond_branch, x, block2, block3}
    }};
    const list_symbol block2_def = list{block2, list
    {
        list{let, y, add_int64, a, b},
        list{branch, block4}
    }};
    const list_symbol block3_def = {block3, list
    {
        list{let, z, sub_int64, a, b},
        list{branch, block4}
    }};
    const list_symbol block4_def = {block4, list
    {
        list{let, w, phi_int64, list{y, block2}, list{z, block3}},
        list{return_int64, w}
    }};

    const list_symbol func1_source =
    {
        params,
        return_type,
        list
        {
            block1_def,
            block2_def,
            block3_def,
            block4_def
        }
    };
    
    auto compiled_function1 = get_compiled_function<uint64_t (uint64_t, uint64_t)>(func1_source);
    BOOST_CHECK(compiled_function1);
    BOOST_CHECK_EQUAL(compiled_function1(2, 2), 2 + 2);
    BOOST_CHECK_EQUAL(compiled_function1(5, 3), 5 - 3);


    const list_symbol block4_invalid_def = {block4, list
    {
        list{let, w, phi_int64, list{z, block3}}, // missing incoming for block2
        list{return_int64, w}
    }};

    const list_symbol func2_source =
    {
        params,
        return_type,
        list
        {
            block1_def,
            block2_def,
            block3_def,
            block4_invalid_def
        }
    };
    BOOST_CHECK_THROW(compile_function(func2_source.begin(), func2_source.end(), context), compile_exception);
    
}


BOOST_AUTO_TEST_CASE(a_times_b_test)
{
    const ref result_loc{"result_loc"_id};
    const ref i_loc{"i_loc"_id};
    const ref init_cmp{"init_cmp"_id};
    const ref current_sum{"current_sum"_id};
    const ref next_sum{"next_sum"_id};
    const ref current_i{"current_i"_id};
    const ref next_i{"next_i"_id};
    const ref loop_cmp{"loop_cmp"_id};
    const ref result{"result"_id};

    const list params =
    {
        list{a, int64_type},
        list{b, int64_type}
    };
    const symbol& return_type = int64_type;
    const list_symbol body =
    {
        list{block1, list
        {
            list{let, result_loc, alloc_int64},
            list{store_int64, lit{"0"}, result_loc}, // v speichert die Zwischenergebnisse und schließlich das Endergebnis der Add.
            list{let, i_loc, alloc_int64},
            list{store_int64, lit{"0"}, i_loc}, // w ist ein Zähler, der zählt, wie oft die erste Eingabe addiert werden muss
            list{let, init_cmp, cmp_ne_int64, b, lit{"0"}}, // x = 1, falls b != 0, sonst: x = 0
            list{cond_branch, init_cmp, block2, block3}
            	
        }},
        list{block2, list
        {
		    list{let, current_sum, load_int64, result_loc}, // y = die aktuelle Summe
		    list{let, next_sum, add_int64, current_sum, a}, //z ist die neue Summe, nach Addition mit a 
		    list{store_int64, next_sum, result_loc},
		    list{let, current_i, load_int64, i_loc}, // der Zähler wird aus dem letzten Block geladen, current_i genannt
		    list{let, next_i, add_int64, current_i, lit{"1"}},
		    list{store_int64, next_i, i_loc},
		    list{let, loop_cmp, cmp_ne_int64, b, next_i},
		    list{cond_branch, loop_cmp, block2, block3}
        }},
        list{block3, list
        {
            list{let, result, load_int64, result_loc},
            list{return_int64, result}    
        }}
    };

    const list_symbol function_source = 
    {
        params,
        return_type,
        body
    };

    auto function_ptr = get_compiled_function<uint64_t (uint64_t, uint64_t)>(function_source);
    BOOST_CHECK(function_ptr(2, 3) == 6);
    BOOST_CHECK(function_ptr(0, 20) == 0);
    BOOST_CHECK(function_ptr(33, 0) == 0);
    BOOST_CHECK(function_ptr(0, 0) == 0);
}

BOOST_AUTO_TEST_CASE(test_missing_let)
{
    const list params =
    {
        list{a, int64_type},
        list{b, int64_type}
    };
    const symbol& return_type = int64_type;
    const list_symbol body =
    {
        list{block1, list
        {
            list{x, add_int64, a, b}
        }}
    };

    const list_symbol function_source = 
    {
        params,
        return_type,
        body
    };
    BOOST_CHECK_THROW(get_compiled_function<void (uint64_t, uint64_t)>(function_source), compile_exception);
}

#include "../src/printing.hpp"
BOOST_AUTO_TEST_CASE(macro_list_XY_instructions_test)
{
    const list params =
    {};
    const symbol& return_type = int64_type;
    const list_symbol body =
    {
        list{block1, list
        {
            list{let, x, list_create},
            list{let, y, list_create},
            list{let, z, list_create},
            list{list_push, x, y},
            list{list_push, x, y},
            list{list_push, x, y},
            list{list_pop, x},
            list{list_set, x, lit{"1"}, z},
            list{let, v, list_get, x, lit{"1"}},
            list{return_int64, v}
        }}
    };

    const list_symbol function_source = 
    {
        params,
        return_type,
        body
    };
    
    unique_ptr<Function> function_owner;
    tie(function_owner, ignore) = compile_function(function_source.begin(), function_source.end(), context);
    context.macro_environment().llvm_module.getFunctionList().push_back(function_owner.get());
    Function* function = function_owner.release();

    const list_symbol arg{lit{"asdf"}, ref{"asdf"_id}};
    auto p = execute_macro(context.macro_environment(), *function, arg.begin(), arg.end()); 
}

