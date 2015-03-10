#include "macro_module.hpp"
#include "symbol.hpp"
#include "compilation_context.hpp"


#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>

#include <boost/variant.hpp>

#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <setjmp.h>

using std::unique_ptr;
using std::make_unique;
using std::vector;
using std::pair;
using std::move;
using std::string;
using std::vector;
using std::size_t;

using llvm::LLVMContext;
using llvm::Function;
using llvm::Module;
using llvm::ExecutionEngine;
using llvm::EngineBuilder;
using llvm::FunctionType;
using llvm::IntegerType;
using llvm::Type;

using boost::variant;
using boost::get;

macro_execution_environment create_macro_environment(llvm::LLVMContext& llvm_context)
{
    auto module_owner = make_unique<Module>("macro module", llvm_context);
    ExecutionEngine* engine = EngineBuilder(module_owner.get()).create();
    assert(engine);
    Module* module = module_owner.release();
    
    Type* int64 = IntegerType::get(llvm_context, 64);
    Type* llvm_void = Type::getVoidTy(llvm_context);

    vector<Type*> params = {};
    FunctionType* sig_int64 = FunctionType::get(int64, params, false);

    params = {int64};
    FunctionType* sig_int64_int64 = FunctionType::get(int64, params, false);
    FunctionType* sig_void_int64 = FunctionType::get(llvm_void, params, false);

    params = {int64, int64};
    FunctionType* sig_int64_2int64 = FunctionType::get(int64, params, false);
    FunctionType* sig_void_2int64 = FunctionType::get(llvm_void, params, false);

    params = {int64, int64, int64};
    FunctionType* sig_void_3int64 = FunctionType::get(int64, params, false);

    
    Function* list_create = Function::Create(sig_int64, Function::InternalLinkage, "macro_list_create", module);
    Function* list_size = Function::Create(sig_int64_int64, Function::InternalLinkage, "macro_list_size", module);
    Function* list_get = Function::Create(sig_int64_2int64, Function::InternalLinkage, "macro_list_get", module);
    Function* list_set = Function::Create(sig_void_3int64, Function::InternalLinkage, "macro_list_set", module);
    Function* list_push = Function::Create(sig_void_2int64, Function::InternalLinkage, "macro_list_push", module);
    Function* list_pop = Function::Create(sig_void_int64, Function::InternalLinkage, "macro_list_pop", module);

    return macro_execution_environment{*module, *engine, *list_create, *list_size, *list_set, *list_get, *list_push, *list_pop};
}

typedef uint64_t symbol_index;

struct indexed_id;
struct indexed_lit;
struct indexed_ref;
struct indexed_list;
struct indexed_macro;
typedef pair<size_t, variant<indexed_id, indexed_lit, indexed_ref, indexed_list, indexed_macro>> indexed_symbol;

struct indexed_id
{
    size_t id;
};
struct indexed_lit
{
    string str;
};
struct indexed_ref
{
    identifier_id_t identifier;
    size_t refered_index;
};
struct indexed_list
{
    vector<size_t> vec;
};
struct indexed_macro
{
};

size_t to_indexed_symbol_impl(const symbol& s, vector<indexed_symbol>& result)
{
    size_t symbol_index = result.size() + 1;
    s.visit(
    [&](const id_symbol& id)
    {
        result.push_back({symbol_index, indexed_id{id.id()}});
    },
    [&](const lit_symbol& lit)
    {
        string str{lit.begin(), lit.end()};
        result.push_back({symbol_index, indexed_lit{move(str)}});
    },
    [&](const ref_symbol& ref)
    {
        result.push_back({symbol_index, indexed_ref{ref.identifier(), 0}});
        if(ref.refered())
        {
            indexed_ref& data = boost::get<indexed_ref>(result.back().second);
            data.refered_index = to_indexed_symbol_impl(*ref.refered(), result);
        }
    },
    [&](const list_symbol& list)
    {
        result.push_back({symbol_index, indexed_list{}});
        indexed_list& data = boost::get<indexed_list>(result.back().second);
        data.vec.reserve(list.size());
        for(const symbol& child : list)
            data.vec.push_back(to_indexed_symbol_impl(child, result));
    },
    [&](const macro_symbol&)
    {
        result.push_back({symbol_index, indexed_macro{}});
    });
    
    return symbol_index;
}

// TODO: these 2 functions assume non-cyclic symbols
vector<indexed_symbol> to_indexed_symbol(list_symbol::const_iterator begin, list_symbol::const_iterator end)
{
    vector<indexed_symbol> result;
    list_symbol top_level_list{vector<any_symbol>{begin, end}};
    to_indexed_symbol_impl(top_level_list, result);
    return result;
}

any_symbol to_symbol_impl(symbol_index index, const vector<indexed_symbol>& indexed_symbols, vector<unique_ptr<any_symbol>>& symbol_store)
{
    return visit<any_symbol>(indexed_symbols[index - 1].second,
    [&](const indexed_id& id)
    {
        return id_symbol{id.id};
    },
    [&](const indexed_lit& lit)
    {
        return lit_symbol{lit.str};
    },
    [&](const indexed_ref& ref)
    {
        ref_symbol result{ref.identifier};
        if(ref.refered_index)
        {
            symbol_store.push_back(make_unique<any_symbol>(to_symbol_impl(ref.refered_index, indexed_symbols, symbol_store)));
            result.refered(symbol_store.back().get());
        }
        return result;
    },
    [&](const indexed_list& list)
    {
        list_symbol result;
        for(size_t child_index : list.vec)
            result.push_back(to_symbol_impl(child_index, indexed_symbols, symbol_store));
        
        return result; 
    },
    [&](const indexed_macro&)
    {
        return macro_symbol{{}};
    });
}
pair<any_symbol, vector<unique_ptr<any_symbol>>> to_symbol(symbol_index index, const vector<indexed_symbol>& indexed_symbols)
{
    vector<unique_ptr<any_symbol>> symbol_store;
    any_symbol symbol = to_symbol_impl(index, indexed_symbols, symbol_store);
    return {move(symbol), move(symbol_store)};
}


struct macro_execution_data
{
    vector<indexed_symbol> symbols;
    jmp_buf jmp_env;
};

thread_local macro_execution_data execution_data;

extern "C"
{

symbol_index macro_list_create()
{
    symbol_index index = execution_data.symbols.size() + 1;
    execution_data.symbols.push_back({index, indexed_list{}});
    return index;
}
// TODO: checks
uint64_t macro_list_size(symbol_index l)
{
    return get<indexed_list>(execution_data.symbols.at(l - 1).second).vec.size();
}
symbol_index macro_list_get(symbol_index l, uint64_t index)
{
    return get<indexed_list>(execution_data.symbols.at(l - 1).second).vec.at(index);
}
void macro_list_set(symbol_index l, uint64_t index, symbol_index s)
{
    get<indexed_list>(execution_data.symbols.at(l - 1).second).vec.at(index) = s;
}
void macro_list_push(symbol_index l, symbol_index s)
{
    get<indexed_list>(execution_data.symbols.at(l - 1).second).vec.push_back(s);
}
void macro_list_pop(symbol_index l)
{
    get<indexed_list>(execution_data.symbols.at(l - 1).second).vec.pop_back();
}

}


pair<any_symbol, vector<unique_ptr<any_symbol>>> execute_macro(macro_execution_environment& environment, Function& function, list_symbol::const_iterator args_begin, list_symbol::const_iterator args_end)
{
    if(setjmp(execution_data.jmp_env))
    {
    }
    
    execution_data.symbols = to_indexed_symbol(args_begin, args_end);
    auto fptr = (symbol_index (*)(symbol_index)) environment.llvm_engine.getPointerToFunction(&function); 
    
    symbol_index result_symbol = fptr(1);
    return to_symbol(result_symbol, execution_data.symbols);
}

