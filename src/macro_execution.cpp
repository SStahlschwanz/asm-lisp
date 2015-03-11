#include "macro_execution.hpp"
#include "indexed_symbol.hpp"
#include "error/macro_execution_error.hpp"

#include <setjmp.h>
#include <cstdint>

using std::pair;
using std::vector;
using std::size_t;
using std::unique_ptr;
using std::int8_t;

using boost::get;
using boost::blank;

using namespace macro_execution_error;

thread_local struct 
{
    vector<indexed_symbol> symbols;
    jmp_buf jmp_env;
} execution_data;

// TODO: better checks (error reporting)

void check_valid_index(symbol_index si)
{
    if(si == 0 || si > execution_data.symbols.size())
        longjmp(execution_data.jmp_env, 1);
}

template<class T>
T& get_indexed(symbol_index si)
{
    check_valid_index(si);
    if(T* obj = get<T>(&execution_data.symbols[si - 1].second))
        return *obj;
    else
        longjmp(execution_data.jmp_env, 1);
}

extern "C"
{

int8_t macro_is_id(symbol_index s)
{
    check_valid_index(s);
    return get<indexed_id>(&execution_data.symbols[s - 1].second) != nullptr;
}
int8_t macro_is_lit(symbol_index s)
{
    check_valid_index(s);
    return get<indexed_lit>(&execution_data.symbols[s - 1].second) != nullptr;
}
int8_t macro_is_ref(symbol_index s)
{
    check_valid_index(s);
    return get<indexed_ref>(&execution_data.symbols[s - 1].second) != nullptr;
}
int8_t macro_is_list(symbol_index s)
{
    check_valid_index(s);
    return get<indexed_list>(&execution_data.symbols[s - 1].second) != nullptr;
}
int8_t macro_is_macro(symbol_index s)
{
    check_valid_index(s);
    return get<indexed_macro>(&execution_data.symbols[s - 1].second) != nullptr;
}

symbol_index macro_lit_create()
{
    symbol_index index = execution_data.symbols.size() + 1;
    execution_data.symbols.push_back({index, indexed_lit{}});
    return index;
}
uint64_t macro_lit_size(symbol_index l)
{
    return get_indexed<indexed_lit>(l).str.size();
}
int8_t macro_lit_get(symbol_index l, uint64_t index) noexcept
{
    indexed_lit& lit = get_indexed<indexed_lit>(l);
    if(index >= lit.str.size())
        longjmp(execution_data.jmp_env, 1);
    
    return lit.str[index];
}
void macro_lit_set(symbol_index l, uint64_t index, int8_t c)
{
    indexed_lit& lit = get_indexed<indexed_lit>(l);
    if(index >= lit.str.size())
        longjmp(execution_data.jmp_env, 1);
    
    lit.str[index] = c;
}
void macro_lit_push(symbol_index l, int8_t c)
{
    get_indexed<indexed_lit>(l).str.push_back(c);
}
void macro_lit_pop(symbol_index l)
{
    indexed_lit& lit = get_indexed<indexed_lit>(l);
    if(lit.str.empty())
        longjmp(execution_data.jmp_env, 1);
    lit.str.pop_back();
}

symbol_index macro_list_create()
{
    symbol_index index = execution_data.symbols.size() + 1;
    execution_data.symbols.push_back({index, indexed_list{}});
    return index;
}
uint64_t macro_list_size(symbol_index l)
{
    return get_indexed<indexed_list>(l).vec.size();
}
symbol_index macro_list_get(symbol_index l, uint64_t index) noexcept
{
    indexed_list& list = get_indexed<indexed_list>(l);
    if(index >= list.vec.size())
        longjmp(execution_data.jmp_env, 1);
    
    return list.vec[index];
}
void macro_list_set(symbol_index l, uint64_t index, symbol_index s)
{
    check_valid_index(s);
    indexed_list& list = get_indexed<indexed_list>(l);
    if(index >= list.vec.size())
        longjmp(execution_data.jmp_env, 1);
    
    list.vec[index] = s;
}
void macro_list_push(symbol_index l, symbol_index s)
{
    check_valid_index(s);
    get_indexed<indexed_list>(l).vec.push_back(s);
}
void macro_list_pop(symbol_index l)
{
    indexed_list& list = get_indexed<indexed_list>(l);
    if(list.vec.empty())
        longjmp(execution_data.jmp_env, 1);
    list.vec.pop_back();
}

}


pair<any_symbol, vector<unique_ptr<any_symbol>>> execute_macro(size_t (*function_ptr)(size_t), list_symbol::const_iterator args_begin, list_symbol::const_iterator args_end)
{
    if(setjmp(execution_data.jmp_env))
        fatal<id("generic_error")>(blank());
    
    execution_data.symbols = to_indexed_symbol(args_begin, args_end);
    
    symbol_index result_symbol = function_ptr(1);
    check_valid_index(result_symbol);
    return to_symbol(result_symbol, execution_data.symbols);
}

