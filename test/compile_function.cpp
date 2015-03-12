#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_function
#include <boost/test/unit_test.hpp>

#include "../src/compile_function.hpp"
#include "../src/core_unique_ids.hpp"
#include "../src/error/compile_exception.hpp"

#include "state_utils.hpp"
#include "function_building.hpp"

#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>

#include <unordered_map>
#include <utility>
#include <iterator>
#include <string>
#include <csetjmp>

using std::pair;
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

    const id_symbol call_constructor{unique_ids::CALL};
    const list_symbol signature_type = {function_signature, list{int64_type, int64_type}, int64_type};
    type_info signature_type_info = compile_type(signature_type, context.llvm());
    const list_symbol instruction4 = {call_constructor, signature_type};
    const instruction_info got4 = parse_instruction(instruction4, context.llvm());
    BOOST_CHECK(get<instruction_info::call>(got4.kind).type.llvm_type == signature_type_info.llvm_type);
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

const list_symbol add_proc
{
    list // params
    {
        list{a, int64_type},
        list{b, int64_type}
    },
    int64_type, // return type 
    list // body
    {
        list{block1, list
        {
            list{let, x, add_int64, a, b},
            list{return_int64, x}
        }}
    }
};
const list_symbol create_empty_list_proc
{
    list{}, // params
    int64_type, // return type
    list // body
    {
        list{block1, list
        {
            list{let, x, list_create},
            list{return_int64, x}
        }}
    }
};

BOOST_AUTO_TEST_CASE(compile_proc_test)
{
    proc_symbol rt_ct_proc = compile_proc(add_proc.begin(), add_proc.end(), context);
    BOOST_CHECK(rt_ct_proc.ct_function() != nullptr);
    BOOST_CHECK(rt_ct_proc.rt_function() != nullptr);

    proc_symbol ct_proc = compile_proc(create_empty_list_proc.begin(), create_empty_list_proc.end(), context);
    BOOST_CHECK(ct_proc.ct_function() != nullptr);
    BOOST_CHECK(ct_proc.rt_function() == nullptr);
}

BOOST_AUTO_TEST_CASE(call_test)
{
    const proc_symbol called_proc = compile_proc(add_proc.begin(), add_proc.end(), context);

    const ref proc_ref{"proc"_id, &called_proc};

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
            list{let, x, list{call, sig_int64_2int64}, proc_ref, a, b},
            list{return_int64, x}
        }}
    };

    const list_symbol function_source = 
    {
        params,
        return_type,
        body
    };

    auto function_ptr = get_compiled_function<uint64_t (uint64_t, uint64_t)>(function_source);
    BOOST_CHECK_EQUAL(function_ptr(2, 2), 2 + 2);
    BOOST_CHECK_EQUAL(function_ptr(23, 43), 66);
}


BOOST_AUTO_TEST_CASE(missing_let_test)
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
    BOOST_CHECK_THROW(compile_function(function_source.begin(), function_source.end(), context), compile_exception);
}

