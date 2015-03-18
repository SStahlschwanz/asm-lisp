#include "core_module.hpp"

#include "error/core_misc_error.hpp"
#include "core_unique_ids.hpp"
#include "compile_function.hpp"

#include <boost/variant.hpp>

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <utility>

using std::pair;
using std::vector;
using std::string;
using std::unique_ptr;
using std::make_unique;
using std::make_shared;
using std::size_t;
using std::move;
using std::tie;
using std::ignore;

using llvm::Function;
using llvm::FunctionType;
using llvm::Type;
using llvm::IntegerType;
using llvm::dyn_cast;

using boost::blank;

using namespace core_misc_error;

module create_core_module(compilation_context& context)
{
    dynamic_graph node_owner;
    symbol_table exports;

    auto add_symbol = [&](string name, node& n)
    {
        bool was_inserted;
        tie(ignore, was_inserted) = exports.insert({move(name), n});
        assert(was_inserted);
    };
    auto add_id_symbol = [&](string name, size_t id)
    {
        id_node& node = node_owner.create_id(id);
        add_symbol(move(name), node);
    };
    auto add_macro_symbol = [&](const char* name, auto func)
    {
        auto f = make_shared<std::function<macro_node::macro>>(move(func));
        macro_node& m = node_owner.create_macro();
        m.function(move(f));
        add_symbol(move(name), m);
    };
    
    size_t next_unique_id = static_cast<size_t>(unique_ids::FIRST_UNUSED);
    auto unique_func = [next_unique_id](node_range nodes) mutable -> pair<node&, dynamic_graph>
    {
        if(!nodes.empty())
            fatal<id("unique_invalid_argument_number")>(blank());
        dynamic_graph graph;
        id_node& id = graph.create_id(next_unique_id++);
        return {id, move(graph)};
    };
    add_macro_symbol("unique", unique_func);
    
    auto macro_func = [&context](node_range nodes) -> pair<node&, dynamic_graph>
    {
        dynamic_graph graph;
        macro_node& macro = graph.create_macro();
        macro = compile_macro(nodes, context);
        return {macro, move(graph)};
    };
    add_macro_symbol("macro", macro_func);

    auto proc_func = [&context](node_range nodes) -> pair<node&, dynamic_graph>
    {
        dynamic_graph graph;
        proc_node& proc = graph.create_proc();
        proc = compile_proc(nodes, context);
        return {proc, move(graph)};
    };
    add_macro_symbol("proc", proc_func);

    /*
    auto external_func = [&context](list_symbol::const_iterator begin, list_symbol::const_iterator end) -> pair<any_symbol, vector<unique_ptr<any_symbol>>>
    {
        if(distance(begin, end) != 2)
            fatal<id("external_invalid_argument_number")>(blank());
        const lit_symbol& external_name = begin->cast_else<lit_symbol>([&]
        {
            fatal<id("external_invalid_name")>(begin->source());
        });
        type_info type = compile_type(*(begin + 1), context.llvm());

        FunctionType* func_type = dyn_cast<FunctionType>(type.llvm_type);
        if(func_type == nullptr)
            fatal<id("external_not_a_signature_type")>(type.node.source());
        unique_ptr<Function> func_owner{Function::Create(func_type, Function::ExternalLinkage, (string)external_name)};
        // TODO: check for duplicate names
        context.runtime_module().getFunctionList().push_back(func_owner.get());
        Function* func = func_owner.release();
        return {proc_symbol{nullptr, func}, vector<unique_ptr<any_symbol>>{}};
    };
    add_macro_symbol("external", external_func);

    auto main_func = [&context](list_symbol::const_iterator begin, list_symbol::const_iterator end) -> pair<any_symbol, vector<unique_ptr<any_symbol>>>
    {
        proc_symbol p = compile_proc(begin, end, context);
        Function* f = p.rt_function();
        if(f == nullptr)
            fatal<id("main_ct_only_proc")>(blank());

        Type* llvm_int32 = IntegerType::get(context.llvm(), 32);
        FunctionType* main_signature = FunctionType::get(llvm_int32, false);
        f->setName("main");
        f->setLinkage(Function::ExternalLinkage);
        if(f->getFunctionType() != main_signature)
            fatal<id("main_invalid_signature")>(blank());

        return {move(p), vector<unique_ptr<any_symbol>>{}};
    };
    add_macro_symbol("main", main_func);
    */

    add_id_symbol("add", unique_ids::ADD);
    add_id_symbol("sub", unique_ids::SUB);
    add_id_symbol("mul", unique_ids::MUL);
    add_id_symbol("sdiv", unique_ids::SDIV);
    add_id_symbol("alloc", unique_ids::ALLOC);
    add_id_symbol("store", unique_ids::STORE);
    add_id_symbol("load", unique_ids::LOAD);
    add_id_symbol("cond_branch", unique_ids::COND_BRANCH);
    add_id_symbol("cmp", unique_ids::CMP);
    add_id_symbol("return", unique_ids::RETURN);
    add_id_symbol("call", unique_ids::CALL);

    add_id_symbol("is_id", unique_ids::IS_ID);
    add_id_symbol("is_lit", unique_ids::IS_LIT);
    add_id_symbol("is_ref", unique_ids::IS_REF);
    add_id_symbol("is_list", unique_ids::IS_LIST);
    add_id_symbol("is_macro", unique_ids::IS_MACRO);

    add_id_symbol("lit_create", unique_ids::LIT_CREATE);
    add_id_symbol("lit_size", unique_ids::LIT_SIZE);
    add_id_symbol("lit_push", unique_ids::LIT_PUSH);
    add_id_symbol("lit_pop", unique_ids::LIT_POP);
    add_id_symbol("lit_get", unique_ids::LIT_GET);
    add_id_symbol("lit_set", unique_ids::LIT_SET);

    add_id_symbol("list_create", unique_ids::LIST_CREATE);
    add_id_symbol("list_size", unique_ids::LIST_SIZE);
    add_id_symbol("list_push", unique_ids::LIST_PUSH);
    add_id_symbol("list_pop", unique_ids::LIST_POP);
    add_id_symbol("list_get", unique_ids::LIST_GET);
    add_id_symbol("list_set", unique_ids::LIST_SET);

    add_id_symbol("eq", unique_ids::EQ);
    add_id_symbol("ne", unique_ids::NE);
    add_id_symbol("lt", unique_ids::LT);
    add_id_symbol("le", unique_ids::LE);
    add_id_symbol("gt", unique_ids::GT);
    add_id_symbol("ge", unique_ids::GE);

    add_id_symbol("int", unique_ids::INT);
    add_id_symbol("ptr", unique_ids::PTR);

    add_id_symbol("let", unique_ids::LET);

    return {move(node_owner), move(exports)};
}

