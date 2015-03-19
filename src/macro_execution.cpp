#include "macro_execution.hpp"
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
    jmp_buf jmp_env;
} execution_data;

// TODO: better checks (error reporting)
/*
void check_valid_index(symbol_index si)
{
    if(si == 0 || si > execution_data.symbols.size())
        longjmp(execution_data.jmp_env, 1);
}

template<class T>
T& get_indexed(symbol_index si)
{
    check_valid_index(si);
    if(T* obj = get<T>(&execution_data.symbols[si - 1]))
        return *obj;
    else
        longjmp(execution_data.jmp_env, 1);
}
*/
typedef std::size_t symbol_index;

extern "C"
{

bool macro_is_id(symbol_index s)
{
}
bool macro_is_lit(symbol_index s)
{
}
bool macro_is_ref(symbol_index s)
{
}
bool macro_is_list(symbol_index s)
{
}
bool macro_is_macro(symbol_index s)
{
}

symbol_index macro_lit_create()
{
}
uint64_t macro_lit_size(symbol_index l)
{
}
int8_t macro_lit_get(symbol_index l, uint64_t index) noexcept
{
}
void macro_lit_set(symbol_index l, uint64_t index, int8_t c)
{
}
void macro_lit_push(symbol_index l, int8_t c)
{
}
void macro_lit_pop(symbol_index l)
{
}

symbol_index macro_list_create()
{
}
uint64_t macro_list_size(symbol_index l)
{
}
symbol_index macro_list_get(symbol_index l, uint64_t index) noexcept
{
}
void macro_list_set(symbol_index l, uint64_t index, symbol_index s)
{
}
void macro_list_push(symbol_index l, symbol_index s)
{
}
void macro_list_pop(symbol_index l)
{
}

}


std::pair<const node&, dynamic_graph> execute_macro(macro_func* func, node_range args)
{
    if(setjmp(execution_data.jmp_env))
        fatal<id("generic_error")>(blank());
}

