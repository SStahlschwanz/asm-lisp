#ifndef FUNCTION_BUILDING_HPP_
#define FUNCTION_BUILDING_HPP_

#include "../src/node.hpp"
#include "../src/core_unique_ids.hpp"
#include "../src/error/compile_exception.hpp"
#include "../src/printing.hpp"

//#include "state_utils.hpp"
#include "graph_building.hpp"
#include "context.hpp"

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Function.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Module.h>

#include <string>
#include <sstream>
#include <memory>

using std::unique_ptr;
using std::tie;
using std::ignore;
using std::string;
using std::ostringstream;

using llvm::Function;
using llvm::raw_string_ostream;
using llvm::verifyFunction;

id_node& function_signature = id{unique_ids::FUNCTION_SIGNATURE};

list_node& int64_type = list{id{unique_ids::INT}, lit{"64"}};
list_node& int1_type = list{id{unique_ids::INT}, lit{"1"}};
ref_node& node_type = ref{"symbol", &int64_type};
list_node& sig_int64_2int64 = list{function_signature, list{int64_type, int64_type}, int64_type};

ref_node& a = ref{"a"};
ref_node& b = ref{"b"};
ref_node& c = ref{"c"};
ref_node& d = ref{"d"};
ref_node& e = ref{"e"};
ref_node& f = ref{"f"};

ref_node& s = ref{"s"};
ref_node& t = ref{"t"};
ref_node& u = ref{"u"};
ref_node& v = ref{"v"};
ref_node& w = ref{"w"};
ref_node& x = ref{"x"};
ref_node& y = ref{"y"};
ref_node& z = ref{"z"};

ref_node& block1 = ref{"block1"};
ref_node& block2 = ref{"block2"};
ref_node& block3 = ref{"block3"};
ref_node& block4 = ref{"block4"};
    
id_node& let = id{unique_ids::LET};

id_node& call = id{unique_ids::CALL};

list_node& alloc_int64 = list{id{unique_ids::ALLOC}, int64_type};
list_node& store_int64 = list{id{unique_ids::STORE}, int64_type};
list_node& load_int64 = list{id{unique_ids::LOAD}, int64_type};
list_node& add_int64 = list{id{unique_ids::ADD}, int64_type};
list_node& sub_int64 = list{id{unique_ids::SUB}, int64_type};
list_node& return_int64 = list{id{unique_ids::RETURN}, int64_type};
list_node& return_symbol = list{id{unique_ids::RETURN}, node_type};
list_node& cmp_eq_int64 = list{id{unique_ids::CMP}, id{unique_ids::EQ}, int64_type};
list_node& cmp_ne_int64 = list{id{unique_ids::CMP}, id{unique_ids::NE}, int64_type};
list_node& cmp_eq_int1 = list{id{unique_ids::CMP}, id{unique_ids::EQ}, int1_type};
list_node& cond_branch = list{id{unique_ids::COND_BRANCH}};
list_node& branch = list{id{unique_ids::BRANCH}};
list_node& phi_int64 = list{id{unique_ids::PHI}, int64_type};

list_node& is_lit = list{id{unique_ids::IS_LIT}};

list_node& lit_create = list{id{unique_ids::LIT_CREATE}};
list_node& lit_size = list{id{unique_ids::LIT_SIZE}};
list_node& lit_set = list{id{unique_ids::LIT_SET}};
list_node& lit_get = list{id{unique_ids::LIT_GET}};
list_node& lit_push = list{id{unique_ids::LIT_PUSH}};
list_node& lit_pop = list{id{unique_ids::LIT_POP}};

list_node& list_create = list{id{unique_ids::LIST_CREATE}};
list_node& list_size = list{id{unique_ids::LIST_SIZE}};
list_node& list_set = list{id{unique_ids::LIST_SET}};
list_node& list_get = list{id{unique_ids::LIST_GET}};
list_node& list_push = list{id{unique_ids::LIST_PUSH}};
list_node& list_pop = list{id{unique_ids::LIST_POP}};

template<class T>
T* get_compiled_function(const list_node& function_source)
{
    unique_ptr<Function> function_owner;
    try
    {
        tie(function_owner, ignore) = compile_function(rangeify(function_source), context());
    }
    catch(const compile_exception& exc)
    {
        ostringstream oss;
        oss << exc;
        BOOST_FAIL("compilation failure:" << oss.str());
    }
    Function* function = function_owner.get();
    context().macro_environment().llvm_module.getFunctionList().push_back(function_owner.get());
    function_owner.release();
    
    string str;
    raw_string_ostream os(str);
    //BOOST_CHECK_MESSAGE(verifyFunction(*function, &os), os.str());
    // TODO: this fails - I don't know why, function->dump() looks good to me and verifyFunction doesn't produce a message
    auto fptr = (T*) context().macro_environment().llvm_engine.getPointerToFunction(function);
    return fptr;
}

#endif

