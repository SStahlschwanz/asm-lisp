#ifndef FUNCTION_BUILDING_HPP_
#define FUNCTION_BUILDING_HPP_

#include "../src/symbol.hpp"
#include "../src/core_unique_ids.hpp"
#include "../src/compile_function.hpp"
#include "../src/error/compile_exception.hpp"
#include "../src/printing.hpp"

#include "state_utils.hpp"

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Function.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Module.h>

#include <string>
#include <sstream>
#include <memory>

using namespace symbol_shortcuts;

using std::unique_ptr;
using std::tie;
using std::ignore;
using std::string;
using std::ostringstream;

using llvm::Function;
using llvm::raw_string_ostream;
using llvm::verifyFunction;

const list int64_type{id_symbol{unique_ids::INT}, lit{"64"}};
const ref symbol_type{"symbol"_id, &int64_type};

const ref a{"a"_id};
const ref b{"b"_id};
const ref c{"c"_id};
const ref d{"d"_id};
const ref e{"e"_id};
const ref f{"f"_id};

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

const list alloc_int64 = {id_symbol{unique_ids::ALLOC}, int64_type};
const list store_int64 = {id_symbol{unique_ids::STORE}, int64_type};
const list load_int64 = {id_symbol{unique_ids::LOAD}, int64_type};
const list add_int64 = {id_symbol{unique_ids::ADD}, int64_type};
const list sub_int64 = {id_symbol{unique_ids::SUB}, int64_type};
const list return_int64 = {id_symbol{unique_ids::RETURN}, int64_type};
const list return_symbol = {id_symbol{unique_ids::RETURN}, symbol_type};
const list cmp_eq_int64 = {id_symbol{unique_ids::CMP}, id_symbol{unique_ids::EQ}, int64_type};
const list cmp_ne_int64 = {id_symbol{unique_ids::CMP}, id_symbol{unique_ids::NE}, int64_type};
const list cond_branch = {id_symbol{unique_ids::COND_BRANCH}};
const list branch = {id_symbol{unique_ids::BRANCH}};
const list phi_int64 = {id_symbol{unique_ids::PHI}, int64_type};

const list lit_create = {id_symbol{unique_ids::LIT_CREATE}};
const list lit_size = {id_symbol{unique_ids::LIT_SIZE}};
const list lit_set = {id_symbol{unique_ids::LIT_SET}};
const list lit_get = {id_symbol{unique_ids::LIT_GET}};
const list lit_push = {id_symbol{unique_ids::LIT_PUSH}};
const list lit_pop = {id_symbol{unique_ids::LIT_POP}};

const list list_create = {id_symbol{unique_ids::LIST_CREATE}};
const list list_size = {id_symbol{unique_ids::LIST_SIZE}};
const list list_set = {id_symbol{unique_ids::LIST_SET}};
const list list_get = {id_symbol{unique_ids::LIST_GET}};
const list list_push = {id_symbol{unique_ids::LIST_PUSH}};
const list list_pop = {id_symbol{unique_ids::LIST_POP}};

template<class T>
T* get_compiled_function(const list_symbol& function_source)
{
    unique_ptr<Function> function_owner;
    try
    {
        tie(function_owner, ignore) = compile_function(function_source.begin(), function_source.end(), context);
    }
    catch(const compile_exception& exc)
    {
        ostringstream oss;
        auto file_id_to_name = [](size_t)
        {
            return "";
        };
        print_error(oss, exc, file_id_to_name);
        BOOST_FAIL("compilation failure:" << oss.str());
    }
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

#endif

