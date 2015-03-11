#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE macro
#include <boost/test/unit_test.hpp>

#include "../src/macro_execution.hpp"

#include "state_utils.hpp"
#include "function_building.hpp"


typedef size_t macro_function(size_t);

#include <iostream>

BOOST_AUTO_TEST_CASE(list_push_pop)
{
    const list params =
    {
        list{s, symbol_type}
    };
    const symbol& return_type = symbol_type;
    
    const list body =
    {
        list{block1, list
        {
            list{let, a, list_create},
            list{let, b, lit_create},
            list{list_push, a, b},
            list{list_push, a, b},
            list{list_pop, a},
            list{return_symbol, a}
        }}
    };

    const list function_source =
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    list l;
    auto p = execute_macro(func, l.begin(), l.end());
    list expected = {lit{}};
    BOOST_CHECK(structurally_equal(p.first, expected));
}

BOOST_AUTO_TEST_CASE(list_size_test)
{
    const list params =
    {
        list{s, symbol_type}
    };
    const symbol& return_type = symbol_type;
    
    const list body =
    {
        list{block1, list
        {
            list{let, a, list_create},
            list{let, b, lit_create},
            list{list_push, a, b},
            list{list_push, a, b},
            list{list_push, a, b},
            list{list_push, a, b},
            list{let, c, list_size, a},
            list{let, d, cmp_eq_int64, c, lit{"4"}},
            list{cond_branch, d, block2, block3}
        }},
        list{block2, list
        {
            list{let, x, lit_create},
            list{return_symbol, x}
        }},
        list{block3, list
        {
            list{let, y, list_create},
            list{return_symbol, y}
        }}
    };

    const list function_source =
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    list l;
    auto p = execute_macro(func, l.begin(), l.end());
    lit expected = {};
    BOOST_CHECK(structurally_equal(p.first, expected));
}

BOOST_AUTO_TEST_CASE(list_set_get_test)
{
    const list params =
    {
        list{s, symbol_type}
    };
    const symbol& return_type = symbol_type;
    
    const list body =
    {
        list{block1, list
        {
            list{let, a, list_create},
            list{let, b, lit_create},
            list{list_push, a, b},
            list{list_push, a, b},
            list{list_push, a, b},
            list{list_push, a, b},
            list{let, c, list_create},
            list{list_set, a, lit{"2"}, c},
            list{let, d, list_get, a, lit{"2"}},
            list{return_symbol, d}
        }}
    };

    const list function_source =
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    list l;
    auto p = execute_macro(func, l.begin(), l.end());
    list expected = {};
    BOOST_CHECK(structurally_equal(p.first, expected));
}
