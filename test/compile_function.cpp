#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_function
#include <boost/test/unit_test.hpp>

#include "../src/compile_function.hpp"
#include "../src/core_unique_ids.hpp"
#include "../src/error/compile_exception.hpp"

#include "function_building.hpp"
#include "context.hpp"

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

BOOST_AUTO_TEST_CASE(compile_signature_test)
{
    node& params1 = list
    {
        list{a, int64_type},
        list{b, int64_type},
        list{c, int64_type}
    };
    node& return_type1 = int64_type;
    
    unique_ptr<Function> function1;
    unordered_map<string, named_value_info> parameter_table1;
    tie(function1, parameter_table1) = compile_signature(params1, return_type1, context());
    
    BOOST_CHECK(function1 != nullptr);
    BOOST_CHECK_EQUAL(parameter_table1.size(), 3);
    BOOST_CHECK(parameter_table1.count(save<string>(a.identifier())));
    BOOST_CHECK(parameter_table1.count(save<string>(b.identifier())));
    BOOST_CHECK(parameter_table1.count(save<string>(c.identifier())));
    BOOST_CHECK(&parameter_table1.at(save<string>(b.identifier())).llvm_value == (++function1->arg_begin()));
    
    node& params2 = list
    {
        list{a, int64_type},
        list{b, int64_type},
        list{a, int64_type}, // duplicate parameter name
    };
    node& return_type2 = int64_type;
    
    BOOST_CHECK_THROW(compile_signature(params2, return_type2, context()), compile_exception);
}

/*
BOOST_AUTO_TEST_CASE(compile_instruction_test)
{
    Type* llvm_int64 = IntegerType::get(context().llvm(), 64);
    
    id_node& add_constructor = id{unique_ids::ADD};
    list_node& instruction1 = list{add_constructor, int64_type}; 
    instruction_info got1 = parse_instruction(instruction1, context().llvm());
    BOOST_CHECK(get<instruction_info::add>(got1.kind).type.llvm_type == llvm_int64);
    
    id_node div_constructor = id{unique_ids::DIV};
    list_node instruction2 = list{div_constructor, int64_type}; 
    instruction_info got2 = parse_instruction(instruction2, context().llvm());
    BOOST_CHECK(&get<instruction_info::div>(got2.kind).type.llvm_type == llvm_int64);

    id_node cmp_constructor = id{unique_ids::CMP};
    ref_node cmp_ref = ref{"asdfasdf"_id, &cmp_constructor};
    id_node lt = id{unique_ids::LT};
    list_node instruction3 = list{cmp_ref, lt, int64_type};
    instruction_info got3 = parse_instruction(instruction3, context().llvm());
    BOOST_CHECK(get<instruction_info::cmp>(got3.kind).cmp_kind.id() == unique_ids::LT);
    BOOST_CHECK(&get<instruction_info::cmp>(got3.kind).type.llvm_type == llvm_int64);

    id_node call_constructor = id{unique_ids::CALL};
    list_node signature_type = list{function_signature, list{int64_type, int64_type}, int64_type};
    type_info signature_type_info = compile_type(signature_type, context().llvm());
    list_node instruction4 = list{call_constructor, signature_type};
    instruction_info got4 = parse_instruction(instruction4, context().llvm());
    BOOST_CHECK(&get<instruction_info::call>(got4.kind).type.llvm_type == &signature_type_info.llvm_type);
}
*/
BOOST_AUTO_TEST_CASE(store_load_test)
{
    list_node params = list
    {
        list{a, int64_type}
    };
    node& return_type = int64_type;
    list_node body = list
    {
        list{block1, list
        {
            list{let, x, alloc_int64},
            list{store_int64, a, x},
            list{let, y, load_int64, x},
            list{return_int64, y}
        }}
    };

    list_node function_source = list
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
    list_node& params = list
    {
        list{a, int64_type},
        list{b, int64_type},
    };
    
    node& return_type = int64_type;
    
    list_node& body = list
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
    list_node& func_source = list
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
    list_node& params = list
    {
        list{a, int64_type},
        list{b, int64_type},
    };
    
    node& return_type = int64_type;

    list_node& block1_def = list{block1, list
    {
        list{let, x, cmp_eq_int64, a, b},
        list{cond_branch, x, block2, block3}
    }};
    list_node& block2_def = list{block2, list
    {
        list{let, y, add_int64, a, b},
        list{branch, block4}
    }};
    list_node& block3_def = list{block3, list
    {
        list{let, z, sub_int64, a, b},
        list{branch, block4}
    }};
    list_node& block4_def = list{block4, list
    {
        list{let, w, phi_int64, list{y, block2}, list{z, block3}},
        list{return_int64, w}
    }};

    list_node& func1_source = list
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


    list_node& block4_invalid_def = list{block4, list
    {
        list{let, w, phi_int64, list{z, block3}}, // missing incoming for block2
        list{return_int64, w}
    }};

    list_node& func2_source = list
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
    BOOST_CHECK_THROW(compile_function(rangeify(func2_source), context()), compile_exception);
    
}

BOOST_AUTO_TEST_CASE(a_times_b_test)
{
    ref_node& result_loc = ref{"result_loc"};
    ref_node& i_loc = ref{"i_loc"};
    ref_node& init_cmp = ref{"init_cmp"};
    ref_node& current_sum = ref{"current_sum"};
    ref_node& next_sum = ref{"next_sum"};
    ref_node& current_i = ref{"current_i"};
    ref_node& next_i = ref{"next_i"};
    ref_node& loop_cmp = ref{"loop_cmp"};
    ref_node& result = ref{"result"};

    list_node& params = list
    {
        list{a, int64_type},
        list{b, int64_type}
    };
    node& return_type = int64_type;
    list_node& body = list
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

    list_node& function_source = list
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
/*
const list_node add_proc
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
const list_node create_empty_list_proc
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
    proc_node rt_ct_proc = compile_proc(add_proc.begin(), add_proc.end(), context());
    BOOST_CHECK(rt_ct_proc.ct_function() != nullptr);
    BOOST_CHECK(rt_ct_proc.rt_function() != nullptr);

    proc_node ct_proc = compile_proc(create_empty_list_proc.begin(), create_empty_list_proc.end(), context());
    BOOST_CHECK(ct_proc.ct_function() != nullptr);
    BOOST_CHECK(ct_proc.rt_function() == nullptr);
}

BOOST_AUTO_TEST_CASE(call_test)
{
    const proc_node called_proc = compile_proc(add_proc.begin(), add_proc.end(), context());

    const ref proc_ref{"proc"_id, &called_proc};

    const list params =
    {
        list{a, int64_type},
        list{b, int64_type}
    };
    const node& return_type = int64_type;
    const list_node body =
    {
        list{block1, list
        {
            list{let, x, list{call, sig_int64_2int64}, proc_ref, a, b},
            list{return_int64, x}
        }}
    };

    const list_node function_source = 
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
    const node& return_type = int64_type;
    const list_node body =
    {
        list{block1, list
        {
            list{x, add_int64, a, b}
        }}
    };

    const list_node function_source = 
    {
        params,
        return_type,
        body
    };
    BOOST_CHECK_THROW(compile_function(function_source.begin(), function_source.end(), context()), compile_exception);
}
*/

