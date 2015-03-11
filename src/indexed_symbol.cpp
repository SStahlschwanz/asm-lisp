#include "indexed_symbol.hpp"

using std::vector;
using std::pair;
using std::string;
using std::unique_ptr;
using std::make_unique;
using std::move;
using std::size_t;

using boost::get;

size_t to_indexed_symbol_impl(const symbol& s, vector<indexed_symbol>& result)
{
    size_t symbol_index = result.size() + 1;
    s.visit(
    [&](const id_symbol& id)
    {
        result.push_back(indexed_id{id.id()});
    },
    [&](const lit_symbol& lit)
    {
        string str{lit.begin(), lit.end()};
        result.push_back(indexed_lit{move(str)});
    },
    [&](const ref_symbol& ref)
    {
        result.push_back(indexed_ref{ref.identifier(), 0});
        if(ref.refered())
        {
            size_t refered_index = to_indexed_symbol_impl(*ref.refered(), result);
            indexed_ref& data = get<indexed_ref>(result[symbol_index - 1]);
            data.refered_index = refered_index;
        }
    },
    [&](const list_symbol& list)
    {
        result.push_back(indexed_list{});
        indexed_list& data = get<indexed_list>(result.back());
        data.vec.reserve(list.size());
        for(const symbol& child : list)
        {
            size_t child_index = to_indexed_symbol_impl(child, result);
            indexed_list& data = get<indexed_list>(result[symbol_index - 1]);
            data.vec.push_back(child_index);
        }
    },
    [&](const macro_symbol& m)
    {
        result.push_back(indexed_macro{m.function()});
    },
    [&](const proc_symbol& p)
    {
        result.push_back(indexed_proc{p.ct_function(), p.rt_function()});
    });
    
    return symbol_index;
}


vector<indexed_symbol> to_indexed_symbol(list_symbol::const_iterator begin, list_symbol::const_iterator end)
{
    vector<indexed_symbol> result;
    list_symbol top_level_list{vector<any_symbol>{begin, end}};
    to_indexed_symbol_impl(top_level_list, result);
    return result;
}

any_symbol to_symbol_impl(symbol_index index, const vector<indexed_symbol>& indexed_symbols, vector<unique_ptr<any_symbol>>& symbol_store)
{
    return visit<any_symbol>(indexed_symbols[index - 1],
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
    [&](const indexed_macro& m)
    {
        return macro_symbol{m.f};
    },
    [&](const indexed_proc& p)
    {
        return proc_symbol{p.ct_function, p.rt_function};
    });
}


pair<any_symbol, vector<unique_ptr<any_symbol>>> to_symbol(symbol_index index, const vector<indexed_symbol>& indexed_symbols)
{
    vector<unique_ptr<any_symbol>> symbol_store;
    any_symbol symbol = to_symbol_impl(index, indexed_symbols, symbol_store);
    return {move(symbol), move(symbol_store)};
}
