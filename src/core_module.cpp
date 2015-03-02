#include "core_module.hpp"

#include "compile_type.hpp"
#include "symbol.hpp"
#include "error/unique_exception.hpp"
#include "core_unique_ids.hpp"

#include <memory>
#include <utility>

using std::unique_ptr;
using std::make_unique;
using std::size_t;
using std::move;
using namespace unique_exception;

module create_core_module(compilation_context& context)
{
    module core;

    auto add_symbol = [&](const char* name, symbol&& s)
    {
        unique_ptr<any_symbol> any = make_unique<any_symbol>(move(s));
        core.exports[context.identifier_id(name)] = any.get(); 
        core.evaluated_exports.push_back(move(any));
    };
    auto add_id_symbol = [&](const char* name, size_t id)
    {
        add_symbol(name, id_symbol{id});
    };
    


    size_t next_unique_id = static_cast<size_t>(unique_ids::FIRST_UNUSED);
    auto unique_func = [next_unique_id](list_symbol::const_iterator begin, list_symbol::const_iterator end) mutable
    {
        if(begin != end)
            throw invalid_argument_number{};
        return id_symbol{next_unique_id++};
    };
    add_symbol("unique", macro_symbol{unique_func});
    
    add_id_symbol("add", unique_ids::ADD);
    add_id_symbol("sub", unique_ids::SUB);
    add_id_symbol("mul", unique_ids::MUL);
    add_id_symbol("div", unique_ids::DIV);
    add_id_symbol("alloc", unique_ids::ALLOC);
    add_id_symbol("store", unique_ids::STORE);
    add_id_symbol("load", unique_ids::LOAD);
    add_id_symbol("cond_branch", unique_ids::COND_BRANCH);
    add_id_symbol("cmp", unique_ids::CMP);
    add_id_symbol("return", unique_ids::RETURN);
    add_id_symbol("call", unique_ids::CALL);
    add_id_symbol("eq", unique_ids::EQ);
    add_id_symbol("ne", unique_ids::NE);
    add_id_symbol("lt", unique_ids::LT);
    add_id_symbol("le", unique_ids::LE);
    add_id_symbol("gt", unique_ids::GT);
    add_id_symbol("ge", unique_ids::GE);


    auto int_func = [&](list_symbol::const_iterator begin, list_symbol::const_iterator end)
    {
        return compile_int(begin, end, context);
    };
    add_symbol("int", macro_symbol{int_func});

    return core;
}

