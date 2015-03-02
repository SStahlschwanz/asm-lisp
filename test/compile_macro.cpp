#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_macro
#include <boost/test/unit_test.hpp>

#include "../src/compile_macro.hpp"
#include "../src/error/compile_macro_exception.hpp"

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
